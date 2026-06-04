// WavesetNorm - normalizes each waveset to peak `amp`, flattening the
// amplitude envelope into a steady buzz. Mono buffers only.

#include "waveset.hpp"

static InterfaceTable* ft;

struct WavesetNorm : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    double m_readPos;
    double m_phase;
    int m_wavesetLen;
    float m_gain; // per-waveset normalization gain
};

void WavesetNorm_next(WavesetNorm* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    float amp = ZIN0(1);
    float rate = ZIN0(2);
    int numCycles = (int)ZIN0(3);
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
    float gain = unit->m_gain;
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
            float peak = waveset::peakAbs(bufData, sp.start, wavesetLen);
            gain = (peak > 1e-7f) ? (amp / peak) : 0.f;
            phase = 0.0;
        }

        out[s] = gain * waveset::readLin(bufData, frames, readPos + phase);

        phase += rate;
        if (phase >= (double)wavesetLen) {
            readPos += (double)wavesetLen;
            wavesetLen = 0;
        }
    }

    unit->m_readPos = readPos;
    unit->m_phase = phase;
    unit->m_wavesetLen = wavesetLen;
    unit->m_gain = gain;
}

void WavesetNorm_Ctor(WavesetNorm* unit) {
    unit->m_fbufnum = -1e9f;
    unit->m_buf = nullptr;

    int startPos = (int)ZIN0(4);
    unit->m_readPos = (startPos < 0) ? 0.0 : (double)startPos;
    unit->m_phase = 0.0;
    unit->m_wavesetLen = 0;
    unit->m_gain = 0.f;

    SETCALC(WavesetNorm_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetNorm) {
    ft = inTable;
    DefineSimpleUnit(WavesetNorm);
}
