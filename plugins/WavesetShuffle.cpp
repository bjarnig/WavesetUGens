// WavesetShuffle - plays each group of `groupSize` wavesets in a random
// (seeded) order. Only waveset boundaries are stored. Mono buffers only.

#include "waveset.hpp"
#include "SC_RGen.h"

static const int kMaxGroup = 64;

static InterfaceTable* ft;

struct WavesetShuffle : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    int m_start[kMaxGroup];
    int m_len[kMaxGroup];
    int m_order[kMaxGroup];
    int m_count;
    int m_orderPos; // index into m_order; >= m_count => need a new group
    int m_srcPos;
    int m_curStart;
    int m_curLen;
    double m_phase;
    RGen m_rgen;
};

void WavesetShuffle_next(WavesetShuffle* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    int groupSize = (int)ZIN0(1);
    float rate = ZIN0(3);
    int numCycles = (int)ZIN0(4);
    if (groupSize < 1)
        groupSize = 1;
    if (groupSize > kMaxGroup)
        groupSize = kMaxGroup;
    if (numCycles < 1)
        numCycles = 1;
    if (rate <= 0.f)
        rate = 1.f;

    if (!bufData || bufChannels != 1 || bufFrames < 2) {
        ClearUnitOutputs(unit, inNumSamples);
        return;
    }

    int count = unit->m_count;
    int orderPos = unit->m_orderPos;
    int srcPos = unit->m_srcPos;
    int curStart = unit->m_curStart;
    int curLen = unit->m_curLen;
    double phase = unit->m_phase;
    const int frames = (int)bufFrames;

    for (int s = 0; s < inNumSamples; s++) {
        if (curLen <= 0) {
            if (orderPos >= count) {
                int n = 0;
                int pos = srcPos;
                for (; n < groupSize; n++) {
                    waveset::Span sp = waveset::nextWaveset(bufData, frames, pos, numCycles);
                    if (sp.end < 0)
                        break;
                    unit->m_start[n] = sp.start;
                    unit->m_len[n] = sp.end - sp.start;
                    pos = sp.end;
                }
                if (n == 0) {
                    out[s] = 0.f;
                    continue;
                }
                count = n;
                srcPos = pos;
                for (int i = 0; i < count; i++)
                    unit->m_order[i] = i;
                for (int i = count - 1; i > 0; i--) { // Fisher-Yates
                    int j = unit->m_rgen.irand(i + 1);
                    int t = unit->m_order[i];
                    unit->m_order[i] = unit->m_order[j];
                    unit->m_order[j] = t;
                }
                orderPos = 0;
            }
            int w = unit->m_order[orderPos];
            curStart = unit->m_start[w];
            curLen = unit->m_len[w];
            phase = 0.0;
        }

        out[s] = waveset::readLin(bufData, frames, (double)curStart + phase);

        phase += rate;
        if (phase >= (double)curLen) {
            orderPos++;
            curLen = 0;
        }
    }

    unit->m_count = count;
    unit->m_orderPos = orderPos;
    unit->m_srcPos = srcPos;
    unit->m_curStart = curStart;
    unit->m_curLen = curLen;
    unit->m_phase = phase;
}

void WavesetShuffle_Ctor(WavesetShuffle* unit) {
    unit->m_fbufnum = -1e9f;
    unit->m_buf = nullptr;

    int startPos = (int)ZIN0(5);
    unit->m_srcPos = (startPos < 0) ? 0 : startPos;
    unit->m_count = 0;
    unit->m_orderPos = 0; // 0 >= 0 => first block detects a group
    unit->m_curStart = 0;
    unit->m_curLen = 0;
    unit->m_phase = 0.0;
    unit->m_rgen.init((uint32)(int)ZIN0(2)); // seed

    SETCALC(WavesetShuffle_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetShuffle) {
    ft = inTable;
    DefineSimpleUnit(WavesetShuffle);
}
