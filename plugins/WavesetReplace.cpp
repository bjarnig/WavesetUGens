// WavesetReplace - the strongest wavecycle in each group of `cyclecnt` replaces
// the others (CDP REPLACE). Each slot keeps its original length (timing
// preserved); the strongest shape is read into it. Mono buffers only.

#include "waveset.hpp"

static InterfaceTable* ft;

struct WavesetReplace : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    int m_start[waveset::kMaxGroup];
    int m_len[waveset::kMaxGroup];
    int m_count;
    int m_slotIdx; // >= m_count => need a new group
    int m_strongStart;
    int m_strongLen;
    int m_srcPos;
    int m_curLen; // length of the current slot
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
    int slotIdx = unit->m_slotIdx;
    int strongStart = unit->m_strongStart;
    int strongLen = unit->m_strongLen;
    int srcPos = unit->m_srcPos;
    int curLen = unit->m_curLen;
    double phase = unit->m_phase;
    const int frames = (int)bufFrames;

    for (int s = 0; s < inNumSamples; s++) {
        if (curLen <= 0) {
            if (slotIdx >= count) {
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
                float bestPk = -1.f;
                for (int i = 0; i < n; i++) {
                    float pk = waveset::peakAbs(bufData, unit->m_start[i], unit->m_len[i]);
                    if (pk > bestPk) {
                        bestPk = pk;
                        best = i;
                    }
                }
                strongStart = unit->m_start[best];
                strongLen = unit->m_len[best];
                slotIdx = 0;
            }
            curLen = unit->m_len[slotIdx]; // slot keeps its original duration
            phase = 0.0;
        }

        double p = phase / (double)curLen; // 0..1 across the slot
        out[s] = waveset::readLin(bufData, frames, (double)strongStart + p * (double)strongLen);

        phase += rate;
        if (phase >= (double)curLen) {
            slotIdx++;
            curLen = 0;
        }
    }

    unit->m_count = count;
    unit->m_slotIdx = slotIdx;
    unit->m_strongStart = strongStart;
    unit->m_strongLen = strongLen;
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
    unit->m_slotIdx = 0; // 0 >= 0 => first block detects a group
    unit->m_strongStart = 0;
    unit->m_strongLen = 0;
    unit->m_curLen = 0;
    unit->m_phase = 0.0;

    SETCALC(WavesetReplace_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetReplace) {
    ft = inTable;
    DefineSimpleUnit(WavesetReplace);
}
