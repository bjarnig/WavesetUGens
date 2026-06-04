// WavesetDelete - keeps one wavecycle per group of `cyclecnt`, dropping the
// rest (CDP DELETE), time-contracting. mode 1: keep first; 2: keep loudest;
// 3: drop the weakest (keep the rest). Mono buffers only.

#include "waveset.hpp"

static InterfaceTable* ft;

struct WavesetDelete : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    int m_start[waveset::kMaxGroup];
    int m_len[waveset::kMaxGroup];
    int m_play[waveset::kMaxGroup]; // indices to play, in order
    int m_playCount;
    int m_playIdx; // >= m_playCount => need a new group
    int m_srcPos;
    int m_curStart;
    int m_curLen;
    double m_phase;
};

void WavesetDelete_next(WavesetDelete* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    int mode = (int)ZIN0(1);
    int cyclecnt = (int)ZIN0(2);
    float rate = ZIN0(3);
    if (mode < 1)
        mode = 1;
    if (mode > 3)
        mode = 3;
    if (cyclecnt < 2)
        cyclecnt = 2;
    if (cyclecnt > waveset::kMaxGroup)
        cyclecnt = waveset::kMaxGroup;
    if (rate <= 0.f)
        rate = 1.f;

    if (!bufData || bufChannels != 1 || bufFrames < 2) {
        ClearUnitOutputs(unit, inNumSamples);
        return;
    }

    int playCount = unit->m_playCount;
    int playIdx = unit->m_playIdx;
    int srcPos = unit->m_srcPos;
    int curStart = unit->m_curStart;
    int curLen = unit->m_curLen;
    double phase = unit->m_phase;
    const int frames = (int)bufFrames;

    for (int s = 0; s < inNumSamples; s++) {
        if (curLen <= 0) {
            if (playIdx >= playCount) {
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
                srcPos = pos;

                if (mode == 1) { // keep first
                    unit->m_play[0] = 0;
                    playCount = 1;
                } else if (mode == 2) { // keep loudest
                    int best = 0;
                    float bestPk = -1.f;
                    for (int i = 0; i < n; i++) {
                        float pk = waveset::peakAbs(bufData, unit->m_start[i], unit->m_len[i]);
                        if (pk > bestPk) {
                            bestPk = pk;
                            best = i;
                        }
                    }
                    unit->m_play[0] = best;
                    playCount = 1;
                } else { // mode 3: drop weakest, keep the rest in order
                    int worst = 0;
                    float worstPk = 1e30f;
                    for (int i = 0; i < n; i++) {
                        float pk = waveset::peakAbs(bufData, unit->m_start[i], unit->m_len[i]);
                        if (pk < worstPk) {
                            worstPk = pk;
                            worst = i;
                        }
                    }
                    int pc = 0;
                    for (int i = 0; i < n; i++)
                        if (i != worst)
                            unit->m_play[pc++] = i;
                    playCount = pc;
                }
                playIdx = 0;
            }
            int w = unit->m_play[playIdx];
            curStart = unit->m_start[w];
            curLen = unit->m_len[w];
            phase = 0.0;
        }

        out[s] = waveset::readLin(bufData, frames, (double)curStart + phase);

        phase += rate;
        if (phase >= (double)curLen) {
            playIdx++;
            curLen = 0;
        }
    }

    unit->m_playCount = playCount;
    unit->m_playIdx = playIdx;
    unit->m_srcPos = srcPos;
    unit->m_curStart = curStart;
    unit->m_curLen = curLen;
    unit->m_phase = phase;
}

void WavesetDelete_Ctor(WavesetDelete* unit) {
    unit->m_fbufnum = -1e9f;
    unit->m_buf = nullptr;

    int startPos = (int)ZIN0(4);
    unit->m_srcPos = (startPos < 0) ? 0 : startPos;
    unit->m_playCount = 0;
    unit->m_playIdx = 0; // 0 >= 0 => first block detects a group
    unit->m_curStart = 0;
    unit->m_curLen = 0;
    unit->m_phase = 0.0;

    SETCALC(WavesetDelete_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetDelete) {
    ft = inTable;
    DefineSimpleUnit(WavesetDelete);
}
