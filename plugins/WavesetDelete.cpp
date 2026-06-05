// WavesetDelete - time-contracts by dropping wavecycles (CDP DELETE). Loudness
// is the sum of |samples| per cycle. Mono buffers only.
//   mode 1 (strict order): keep 1 cycle, then delete the next `cyclecnt`.
//   mode 2 (keep loudest):  of `cyclecnt` cycles, output only the loudest.
//   mode 3 (drop weakest):  of `cyclecnt` cycles, output all but the quietest.

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
    if (cyclecnt < 1)
        cyclecnt = 1;
    if (mode == 3 && cyclecnt < 2)
        cyclecnt = 2; // need >= 2 to drop one and keep some
    // mode 1 keeps 1 then deletes cyclecnt -> group is cyclecnt + 1
    int groupSize = (mode == 1) ? (cyclecnt + 1) : cyclecnt;
    if (groupSize > waveset::kMaxGroup)
        groupSize = waveset::kMaxGroup;
    if (rate <= 0.f)
        rate = 1.f;

    int playCount = unit->m_playCount;
    int playIdx = unit->m_playIdx;
    int srcPos = unit->m_srcPos;
    int curStart = unit->m_curStart;
    int curLen = unit->m_curLen;
    double phase = unit->m_phase;
    const int frames = (int)bufFrames;

    if (!bufData || bufChannels != 1 || bufFrames < 2) {
        ClearUnitOutputs(unit, inNumSamples);
        return;
    }

    for (int s = 0; s < inNumSamples; s++) {
        if (curLen <= 0) {
            if (playIdx >= playCount) {
                int n = 0;
                int pos = srcPos;
                for (; n < groupSize; n++) {
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

                if (mode == 1) { // keep the first cycle, delete the rest
                    unit->m_play[0] = 0;
                    playCount = 1;
                } else if (mode == 2) { // keep the loudest
                    int best = 0;
                    double bestL = -1.0;
                    for (int i = 0; i < n; i++) {
                        double l = waveset::sumAbs(bufData, unit->m_start[i], unit->m_len[i]);
                        if (l > bestL) { bestL = l; best = i; }
                    }
                    unit->m_play[0] = best;
                    playCount = 1;
                } else { // mode 3: drop the quietest, keep the rest in order
                    int worst = 0;
                    double worstL = 1e30;
                    for (int i = 0; i < n; i++) {
                        double l = waveset::sumAbs(bufData, unit->m_start[i], unit->m_len[i]);
                        if (l < worstL) { worstL = l; worst = i; }
                    }
                    int pc = 0;
                    for (int i = 0; i < n; i++)
                        if (i != worst)
                            unit->m_play[pc++] = i;
                    playCount = pc;
                }
                playIdx = 0;
                if (playCount < 1) { // degenerate (e.g. n == 1 in mode 3): skip
                    out[s] = 0.f;
                    continue;
                }
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
