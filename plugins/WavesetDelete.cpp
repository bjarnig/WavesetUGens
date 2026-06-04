// WavesetDelete - plays `keep` wavesets, then drops `drop` (read pointer
// advances, nothing emitted); kept wavesets splice back-to-back. Mono only.

#include "waveset.hpp"

static InterfaceTable* ft;

struct WavesetDelete : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    double m_readPos;
    double m_phase;
    int m_wavesetLen;
    int m_played; // consecutive played wavesets since the last drop
};

void WavesetDelete_next(WavesetDelete* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    int keep = (int)ZIN0(1);
    int drop = (int)ZIN0(2);
    float rate = ZIN0(3);
    int numCycles = (int)ZIN0(4);
    if (keep < 1)
        keep = 1;
    if (drop < 0)
        drop = 0;
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
    int played = unit->m_played;
    const int frames = (int)bufFrames;

    for (int s = 0; s < inNumSamples; s++) {
        if (wavesetLen <= 0) {
            // After `keep` played wavesets, advance past `drop` wavesets.
            if (played >= keep) {
                for (int d = 0; d < drop; d++) {
                    if ((int)readPos >= frames)
                        readPos = 0.0;
                    int e = waveset::findEnd(bufData, frames, (int)readPos, numCycles);
                    if (e < 0) {
                        readPos = 0.0;
                        e = waveset::findEnd(bufData, frames, 0, numCycles);
                        if (e < 0)
                            break;
                    }
                    readPos = (double)e;
                }
                played = 0;
            }
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
            played++;
        }

        out[s] = waveset::readLin(bufData, frames, readPos + phase);

        phase += rate;
        if (phase >= (double)wavesetLen) {
            readPos += (double)wavesetLen;
            wavesetLen = 0;
        }
    }

    unit->m_readPos = readPos;
    unit->m_phase = phase;
    unit->m_wavesetLen = wavesetLen;
    unit->m_played = played;
}

void WavesetDelete_Ctor(WavesetDelete* unit) {
    unit->m_fbufnum = -1e9f;
    unit->m_buf = nullptr;

    int startPos = (int)ZIN0(5);
    unit->m_readPos = (startPos < 0) ? 0.0 : (double)startPos;
    unit->m_phase = 0.0;
    unit->m_wavesetLen = 0;
    unit->m_played = 0;

    SETCALC(WavesetDelete_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetDelete) {
    ft = inTable;
    DefineSimpleUnit(WavesetDelete);
}
