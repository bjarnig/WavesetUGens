// WavesetMultiply - plays each waveset `factor` times at factor x speed, so it
// fills its original span: pitch up by `factor`, duration preserved. Mono only.

#include "waveset.hpp"

static InterfaceTable* ft;

struct WavesetMultiply : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    double m_readPos;
    double m_phase;
    int m_wavesetLen;
    int m_repeatsLeft;
};

void WavesetMultiply_next(WavesetMultiply* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    int factor = (int)ZIN0(1);
    float rate = ZIN0(2);
    int numCycles = (int)ZIN0(3);
    if (factor < 1)
        factor = 1;
    if (numCycles < 1)
        numCycles = 1;
    if (rate <= 0.f)
        rate = 1.f;
    const double effRate = (double)rate * (double)factor;

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
            int end = waveset::findEnd(bufData, frames, (int)readPos, numCycles);
            if (end < 0) {
                readPos = 0.0;
                end = waveset::findEnd(bufData, frames, 0, numCycles);
                if (end < 0) {
                    out[s] = 0.f;
                    continue;
                }
            }
            wavesetLen = end - (int)readPos;
            repeatsLeft = factor;
            phase = 0.0;
        }

        out[s] = waveset::readLin(bufData, frames, readPos + phase);

        phase += effRate;
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

void WavesetMultiply_Ctor(WavesetMultiply* unit) {
    unit->m_fbufnum = -1e9f;
    unit->m_buf = nullptr;

    int startPos = (int)ZIN0(4);
    unit->m_readPos = (startPos < 0) ? 0.0 : (double)startPos;
    unit->m_phase = 0.0;
    unit->m_wavesetLen = 0;
    unit->m_repeatsLeft = 0;

    SETCALC(WavesetMultiply_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetMultiply) {
    ft = inTable;
    DefineSimpleUnit(WavesetMultiply);
}
