// WavesetReplace - the loudest wavecycle in each group of `cyclecnt` (by sum of
// |samples|) replaces the others: the group becomes that cycle repeated
// `cyclecnt` times, verbatim (CDP REPLACE / do_cycle_loudrep). Mono buffers only.

#include "waveset.hpp"

static InterfaceTable* ft;

struct WavesetReplace : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    int m_start[waveset::kMaxGroup];
    int m_len[waveset::kMaxGroup];
    int m_count; // wavecycles in the group (== repeats emitted)
    int m_repIdx; // >= m_count => need a new group
    int m_loudStart;
    int m_loudLen;
    int m_srcPos;
    int m_curLen;
    double m_phase;
};

void WavesetReplace_next(WavesetReplace* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    int cyclecnt = (int)ZIN0(1);
    float rate = ZIN0(2);
    if (cyclecnt < 1)
        cyclecnt = 1;
    if (cyclecnt > waveset::kMaxGroup)
        cyclecnt = waveset::kMaxGroup;
    if (rate <= 0.f)
        rate = 1.f;

    if (!bufData || bufChannels != 1 || bufFrames < 2) {
        ClearUnitOutputs(unit, inNumSamples);
        return;
    }

    int count = unit->m_count;
    int repIdx = unit->m_repIdx;
    int loudStart = unit->m_loudStart;
    int loudLen = unit->m_loudLen;
    int srcPos = unit->m_srcPos;
    int curLen = unit->m_curLen;
    double phase = unit->m_phase;
    const int frames = (int)bufFrames;

    for (int s = 0; s < inNumSamples; s++) {
        if (curLen <= 0) {
            if (repIdx >= count) {
                int n = 0;
                int pos = srcPos;
                for (; n < cyclecnt; n++) {
                    waveset::Span sp = waveset::nextWaveset(bufData, frames, pos, 1);
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
                int best = 0;
                double bestL = -1.0;
                for (int i = 0; i < n; i++) {
                    double l = waveset::sumAbs(bufData, unit->m_start[i], unit->m_len[i]);
                    if (l > bestL) {
                        bestL = l;
                        best = i;
                    }
                }
                loudStart = unit->m_start[best];
                loudLen = unit->m_len[best];
                repIdx = 0;
            }
            curLen = loudLen;
            phase = 0.0;
        }

        out[s] = waveset::readLin(bufData, frames, (double)loudStart + phase);

        phase += rate;
        if (phase >= (double)curLen) {
            repIdx++;
            curLen = 0;
        }
    }

    unit->m_count = count;
    unit->m_repIdx = repIdx;
    unit->m_loudStart = loudStart;
    unit->m_loudLen = loudLen;
    unit->m_srcPos = srcPos;
    unit->m_curLen = curLen;
    unit->m_phase = phase;
}

void WavesetReplace_Ctor(WavesetReplace* unit) {
    unit->m_fbufnum = -1e9f;
    unit->m_buf = nullptr;

    int startPos = (int)ZIN0(3);
    unit->m_srcPos = (startPos < 0) ? 0 : startPos;
    unit->m_count = 0;
    unit->m_repIdx = 0; // 0 >= 0 => first block detects a group
    unit->m_loudStart = 0;
    unit->m_loudLen = 0;
    unit->m_curLen = 0;
    unit->m_phase = 0.0;

    SETCALC(WavesetReplace_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetReplace) {
    ft = inTable;
    DefineSimpleUnit(WavesetReplace);
}
