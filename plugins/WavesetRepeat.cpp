// WavesetRepeat - CDP-style waveset repeat as a buffer-reading UGen.
// Each waveset (numCycles wavecycles between zero crossings) is replayed
// `repeats` times before the read pointer advances. Mono buffers only.

#include "SC_PlugIn.h"

static InterfaceTable* ft;

struct WavesetRepeat : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    double m_readPos; // start of current waveset, in frames
    double m_phase; // playback offset within the waveset, in frames
    int m_wavesetLen; // 0 => none loaded
    int m_repeatsLeft;
};

// Frame index one past `numCycles` full wavecycles from `start`, or -1 if the
// scan runs off the end. Called once per emitted waveset, not per sample.
static inline int findWavesetEnd(const float* data, int frames, int start, int numCycles) {
    int i = start;
    for (int n = 0; n < numCycles; n++) {
        if (data[i] >= 0.f) {
            while (i < frames && data[i] >= 0.f)
                i++;
            while (i < frames && data[i] <= 0.f)
                i++;
        } else {
            while (i < frames && data[i] <= 0.f)
                i++;
            while (i < frames && data[i] >= 0.f)
                i++;
        }
        if (i >= frames)
            return -1;
    }
    return i;
}

void WavesetRepeat_next(WavesetRepeat* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    int repeats = (int)ZIN0(1);
    float rate = ZIN0(2);
    int numCycles = (int)ZIN0(3);
    if (repeats < 1)
        repeats = 1;
    if (numCycles < 1)
        numCycles = 1;
    if (rate <= 0.f)
        rate = 1.f;

    if (!bufData || bufChannels != 1 || bufFrames < 2) {
        ClearUnitOutputs(unit, inNumSamples);
        return;
    }

    double readPos = unit->m_readPos;
    double phase = unit->m_phase;
    int wavesetLen = unit->m_wavesetLen;
    int repeatsLeft = unit->m_repeatsLeft;
    const int frames = (int)bufFrames;

    for (int s = 0; s < inNumSamples; s++) {
        if (wavesetLen <= 0) {
            if ((int)readPos >= frames)
                readPos = 0.0;
            int end = findWavesetEnd(bufData, frames, (int)readPos, numCycles);
            if (end < 0) {
                readPos = 0.0;
                end = findWavesetEnd(bufData, frames, 0, numCycles);
                if (end < 0) {
                    out[s] = 0.f;
                    continue;
                }
            }
            wavesetLen = end - (int)readPos;
            repeatsLeft = repeats;
            phase = 0.0;
        }

        double rp = readPos + phase;
        int i0 = (int)rp;
        double frac = rp - (double)i0;
        float a = bufData[i0];
        float b = (i0 + 1 < frames) ? bufData[i0 + 1] : bufData[i0];
        out[s] = (float)(a + frac * (b - a));

        phase += rate;
        if (phase >= (double)wavesetLen) {
            repeatsLeft--;
            if (repeatsLeft > 0) {
                phase -= (double)wavesetLen;
            } else {
                readPos += (double)wavesetLen;
                wavesetLen = 0;
            }
        }
    }

    unit->m_readPos = readPos;
    unit->m_phase = phase;
    unit->m_wavesetLen = wavesetLen;
    unit->m_repeatsLeft = repeatsLeft;
}

void WavesetRepeat_Ctor(WavesetRepeat* unit) {
    unit->m_fbufnum = -1e9f; // force GET_BUF to resolve the buffer
    unit->m_buf = nullptr;

    int startPos = (int)ZIN0(4);
    unit->m_readPos = (startPos < 0) ? 0.0 : (double)startPos;
    unit->m_phase = 0.0;
    unit->m_wavesetLen = 0;
    unit->m_repeatsLeft = 0;

    SETCALC(WavesetRepeat_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetRepeat) {
    ft = inTable;
    DefineSimpleUnit(WavesetRepeat);
}
