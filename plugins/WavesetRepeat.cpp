// WavesetRepeat - CDP-style waveset repeat as a buffer-reading UGen.
// Replays each waveset (cyclecnt wavecycles between zero crossings) `multiplier`
// times before advancing. Mono buffers only.

#include "waveset.hpp"

static InterfaceTable* ft;

struct WavesetRepeat : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    double m_readPos; // start of current waveset, in frames
    double m_phase;
    int m_wavesetLen; // 0 => none loaded
    int m_repeatsLeft;
};

void WavesetRepeat_next(WavesetRepeat* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    int multiplier = (int)ZIN0(1);
    float rate = ZIN0(2);
    int cyclecnt = (int)ZIN0(3);
    if (multiplier < 1)
        multiplier = 1;
    if (cyclecnt < 1)
        cyclecnt = 1;
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
            int end = waveset::findEnd(bufData, frames, (int)readPos, cyclecnt);
            if (end < 0) {
                readPos = 0.0;
                end = waveset::findEnd(bufData, frames, 0, cyclecnt);
                if (end < 0) {
                    out[s] = 0.f;
                    continue;
                }
            }
            wavesetLen = end - (int)readPos;
            repeatsLeft = multiplier;
            phase = 0.0;
        }

        out[s] = waveset::readLin(bufData, frames, readPos + phase);

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
