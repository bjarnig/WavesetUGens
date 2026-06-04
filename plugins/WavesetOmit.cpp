// WavesetOmit - silences `omit` out of every `outOf` wavesets, replacing them
// with silence (timing preserved). Mono buffers only.

#include "waveset.hpp"

static InterfaceTable* ft;

struct WavesetOmit : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    double m_readPos;
    double m_phase;
    int m_wavesetLen;
    int m_groupIndex; // position in the keep+skip cycle
    int m_silent; // current waveset omitted?
};

void WavesetOmit_next(WavesetOmit* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    int omit = (int)ZIN0(1);
    int outOf = (int)ZIN0(2);
    float rate = ZIN0(3);
    int cyclecnt = (int)ZIN0(4);
    if (outOf < 1)
        outOf = 1;
    if (omit < 0)
        omit = 0;
    if (omit > outOf)
        omit = outOf;
    int keep = outOf - omit; // play `keep`, silence the trailing `omit`
    int period = outOf;
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
    int groupIndex = unit->m_groupIndex;
    int silent = unit->m_silent;
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
            phase = 0.0;
            silent = (groupIndex >= keep);
            groupIndex = (groupIndex + 1) % period;
        }

        out[s] = silent ? 0.f : waveset::readLin(bufData, frames, readPos + phase);

        phase += rate;
        if (phase >= (double)wavesetLen) {
            readPos += (double)wavesetLen;
            wavesetLen = 0;
        }
    }

    unit->m_readPos = readPos;
    unit->m_phase = phase;
    unit->m_wavesetLen = wavesetLen;
    unit->m_groupIndex = groupIndex;
    unit->m_silent = silent;
}

void WavesetOmit_Ctor(WavesetOmit* unit) {
    unit->m_fbufnum = -1e9f;
    unit->m_buf = nullptr;

    int startPos = (int)ZIN0(5);
    unit->m_readPos = (startPos < 0) ? 0.0 : (double)startPos;
    unit->m_phase = 0.0;
    unit->m_wavesetLen = 0;
    unit->m_groupIndex = 0;
    unit->m_silent = 0;

    SETCALC(WavesetOmit_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetOmit) {
    ft = inTable;
    DefineSimpleUnit(WavesetOmit);
}
