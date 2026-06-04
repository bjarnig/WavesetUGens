// WavesetReverse - plays each waveset reversed in time. Mono buffers only.

#include "waveset.hpp"

static InterfaceTable* ft;

struct WavesetReverse : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    double m_readPos;
    double m_phase;
    int m_wavesetLen;
};

void WavesetReverse_next(WavesetReverse* unit, int inNumSamples) {
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
            phase = 0.0;
        }

        // read from the end of the waveset backwards
        out[s] = waveset::readLin(bufData, frames, readPos + ((double)wavesetLen - phase));

        phase += rate;
        if (phase >= (double)wavesetLen) {
            readPos += (double)wavesetLen;
            wavesetLen = 0;
        }
    }

    unit->m_readPos = readPos;
    unit->m_phase = phase;
    unit->m_wavesetLen = wavesetLen;
}

void WavesetReverse_Ctor(WavesetReverse* unit) {
    unit->m_fbufnum = -1e9f;
    unit->m_buf = nullptr;

    int startPos = (int)ZIN0(3);
    unit->m_readPos = (startPos < 0) ? 0.0 : (double)startPos;
    unit->m_phase = 0.0;
    unit->m_wavesetLen = 0;

    SETCALC(WavesetReverse_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetReverse) {
    ft = inTable;
    DefineSimpleUnit(WavesetReverse);
}
