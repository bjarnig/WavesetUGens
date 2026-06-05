// WavesetFractal - adds self-similar detail: a group of `scale` wavecycles is
// the "large" shape; a miniature copy of the whole group (scaled by `amp`) is
// added into each constituent cycle (CDP FRACTAL). Length is preserved. Mono.

#include "waveset.hpp"

static InterfaceTable* ft;

struct WavesetFractal : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    int m_start[waveset::kMaxGroup];
    int m_len[waveset::kMaxGroup];
    int m_count;
    int m_groupStart;
    int m_groupLen; // total samples in the group
    int m_cyc; // current cycle within the group
    double m_phase; // 0 .. len of current cycle
    int m_srcPos;
    int m_have;
};

void WavesetFractal_next(WavesetFractal* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    int scale = (int)ZIN0(1);
    float amp = ZIN0(2);
    float rate = ZIN0(3);
    if (scale < 1)
        scale = 1;
    if (scale > waveset::kMaxGroup)
        scale = waveset::kMaxGroup;
    if (rate <= 0.f)
        rate = 1.f;

    if (!bufData || bufChannels != 1 || bufFrames < 2) {
        ClearUnitOutputs(unit, inNumSamples);
        return;
    }

    int count = unit->m_count;
    int groupStart = unit->m_groupStart;
    int groupLen = unit->m_groupLen;
    int cyc = unit->m_cyc;
    double phase = unit->m_phase;
    int srcPos = unit->m_srcPos;
    int have = unit->m_have;
    const int frames = (int)bufFrames;

    for (int s = 0; s < inNumSamples; s++) {
        if (!have) {
            int n = 0, pos = srcPos;
            int total = 0;
            for (; n < scale; n++) {
                waveset::Span sp = waveset::nextWaveset(bufData, frames, pos, 1);
                if (sp.end < 0)
                    break;
                if (n == 0)
                    groupStart = sp.start;
                unit->m_start[n] = sp.start;
                unit->m_len[n] = sp.end - sp.start;
                total += unit->m_len[n];
                pos = sp.end;
            }
            if (n == 0) {
                out[s] = 0.f;
                continue;
            }
            count = n;
            groupLen = total;
            srcPos = pos;
            cyc = 0;
            phase = 0.0;
            have = 1;
        }

        int cstart = unit->m_start[cyc];
        int clen = unit->m_len[cyc];
        double miniPos = (double)groupStart + (phase / (double)clen) * (double)groupLen;
        out[s] = waveset::readLin(bufData, frames, (double)cstart + phase)
            + amp * waveset::readLin(bufData, frames, miniPos);

        phase += rate;
        if (phase >= (double)clen) {
            phase -= (double)clen;
            cyc++;
            if (cyc >= count)
                have = 0;
        }
    }

    unit->m_count = count;
    unit->m_groupStart = groupStart;
    unit->m_groupLen = groupLen;
    unit->m_cyc = cyc;
    unit->m_phase = phase;
    unit->m_srcPos = srcPos;
    unit->m_have = have;
}

void WavesetFractal_Ctor(WavesetFractal* unit) {
    unit->m_fbufnum = -1e9f;
    unit->m_buf = nullptr;
    int startPos = (int)ZIN0(4);
    unit->m_srcPos = (startPos < 0) ? 0 : startPos;
    unit->m_count = 0;
    unit->m_groupStart = 0;
    unit->m_groupLen = 0;
    unit->m_cyc = 0;
    unit->m_phase = 0.0;
    unit->m_have = 0;
    SETCALC(WavesetFractal_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetFractal) {
    ft = inTable;
    DefineSimpleUnit(WavesetFractal);
}
