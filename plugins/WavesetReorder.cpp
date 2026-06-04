// WavesetReorder - plays each group of `groupSize` wavesets in reverse order.
// Only waveset boundaries are stored (audio stays in the buffer). Mono only.

#include "waveset.hpp"

static const int kMaxGroup = 64;

static InterfaceTable* ft;

struct WavesetReorder : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    int m_start[kMaxGroup];
    int m_len[kMaxGroup];
    int m_count; // wavesets in the current group
    int m_idx; // index being played (counts down); < 0 => need a new group
    int m_srcPos; // source scan pointer, persists across groups
    int m_curStart;
    int m_curLen;
    double m_phase;
};

void WavesetReorder_next(WavesetReorder* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    int groupSize = (int)ZIN0(1);
    float rate = ZIN0(2);
    int numCycles = (int)ZIN0(3);
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
    int idx = unit->m_idx;
    int srcPos = unit->m_srcPos;
    int curStart = unit->m_curStart;
    int curLen = unit->m_curLen;
    double phase = unit->m_phase;
    const int frames = (int)bufFrames;

    for (int s = 0; s < inNumSamples; s++) {
        if (curLen <= 0) {
            if (idx < 0) {
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
                idx = count - 1; // reverse order
            }
            curStart = unit->m_start[idx];
            curLen = unit->m_len[idx];
            phase = 0.0;
        }

        out[s] = waveset::readLin(bufData, frames, (double)curStart + phase);

        phase += rate;
        if (phase >= (double)curLen) {
            idx--;
            curLen = 0; // next iteration loads idx, or detects a new group if idx < 0
        }
    }

    unit->m_count = count;
    unit->m_idx = idx;
    unit->m_srcPos = srcPos;
    unit->m_curStart = curStart;
    unit->m_curLen = curLen;
    unit->m_phase = phase;
}

void WavesetReorder_Ctor(WavesetReorder* unit) {
    unit->m_fbufnum = -1e9f;
    unit->m_buf = nullptr;

    int startPos = (int)ZIN0(4);
    unit->m_srcPos = (startPos < 0) ? 0 : startPos;
    unit->m_count = 0;
    unit->m_idx = -1;
    unit->m_curStart = 0;
    unit->m_curLen = 0;
    unit->m_phase = 0.0;

    SETCALC(WavesetReorder_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetReorder) {
    ft = inTable;
    DefineSimpleUnit(WavesetReorder);
}
