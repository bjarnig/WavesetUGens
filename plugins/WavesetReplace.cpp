// WavesetReplace - replaces each waveset with a sine of the same duration
// (numCycles cycles) and the waveset's peak amplitude. Mono buffers only.

#include "waveset.hpp"
#include <cmath>

static InterfaceTable* ft;

struct WavesetReplace : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    double m_readPos;
    double m_phase;
    int m_wavesetLen;
    float m_peak; // peak amplitude of the loaded waveset
};

void WavesetReplace_next(WavesetReplace* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    float rate = ZIN0(1);
    int numCycles = (int)ZIN0(2);
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
    float peak = unit->m_peak;
    const int frames = (int)bufFrames;

    for (int s = 0; s < inNumSamples; s++) {
        if (wavesetLen <= 0) {
            waveset::Span sp = waveset::nextWaveset(bufData, frames, (int)readPos, numCycles);
            if (sp.end < 0) {
                out[s] = 0.f;
                continue;
            }
            readPos = (double)sp.start;
            wavesetLen = sp.end - sp.start;
            peak = waveset::peakAbs(bufData, sp.start, wavesetLen);
            phase = 0.0;
        }

        double frac = phase / (double)wavesetLen; // 0..1 across the waveset
        out[s] = peak * (float)sin(2.0 * M_PI * (double)numCycles * frac);

        phase += rate;
        if (phase >= (double)wavesetLen) {
            readPos += (double)wavesetLen;
            wavesetLen = 0;
        }
    }

    unit->m_readPos = readPos;
    unit->m_phase = phase;
    unit->m_wavesetLen = wavesetLen;
    unit->m_peak = peak;
}

void WavesetReplace_Ctor(WavesetReplace* unit) {
    unit->m_fbufnum = -1e9f;
    unit->m_buf = nullptr;

    int startPos = (int)ZIN0(3);
    unit->m_readPos = (startPos < 0) ? 0.0 : (double)startPos;
    unit->m_phase = 0.0;
    unit->m_wavesetLen = 0;
    unit->m_peak = 0.f;

    SETCALC(WavesetReplace_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetReplace) {
    ft = inTable;
    DefineSimpleUnit(WavesetReplace);
}
