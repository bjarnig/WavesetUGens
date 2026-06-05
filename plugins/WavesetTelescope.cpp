// WavesetTelescope - superimposes a group of `cyclecnt` wavecycles into a single
// cycle (their average, sampled at matched phase), time-contracting (CDP
// TELESCOPE). The output cycle length is the longest cycle of the group, or the
// mean if `useMean` is set. Mono buffers only.

#include "waveset.hpp"

static InterfaceTable* ft;

struct WavesetTelescope : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    int m_start[waveset::kMaxGroup];
    int m_len[waveset::kMaxGroup];
    int m_count;
    int m_refLen; // output cycle length
    double m_phase; // 0 .. m_refLen
    int m_srcPos;
    int m_have; // a group is loaded?
};

void WavesetTelescope_next(WavesetTelescope* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    int cyclecnt = (int)ZIN0(1);
    int useMean = (int)ZIN0(2);
    float rate = ZIN0(3);
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
    int refLen = unit->m_refLen;
    double phase = unit->m_phase;
    int srcPos = unit->m_srcPos;
    int have = unit->m_have;
    const int frames = (int)bufFrames;

    for (int s = 0; s < inNumSamples; s++) {
        if (!have) {
            int n = 0, pos = srcPos;
            long lenSum = 0, lenMax = 0;
            for (; n < cyclecnt; n++) {
                waveset::Span sp = waveset::nextWaveset(bufData, frames, pos, 1);
                if (sp.end < 0)
                    break;
                int l = sp.end - sp.start;
                unit->m_start[n] = sp.start;
                unit->m_len[n] = l;
                lenSum += l;
                if (l > lenMax)
                    lenMax = l;
                pos = sp.end;
            }
            if (n == 0) {
                out[s] = 0.f;
                continue;
            }
            count = n;
            srcPos = pos;
            refLen = useMean ? (int)((lenSum + n / 2) / n) : (int)lenMax;
            if (refLen < 1)
                refLen = 1;
            phase = 0.0;
            have = 1;
        }

        double ratio = phase / (double)refLen; // 0..1
        double sum = 0.0;
        for (int i = 0; i < count; i++)
            sum += waveset::readLin(bufData, frames, (double)unit->m_start[i] + ratio * (double)unit->m_len[i]);
        out[s] = (float)(sum / (double)count);

        phase += rate;
        if (phase >= (double)refLen)
            have = 0; // next group
    }

    unit->m_count = count;
    unit->m_refLen = refLen;
    unit->m_phase = phase;
    unit->m_srcPos = srcPos;
    unit->m_have = have;
}

void WavesetTelescope_Ctor(WavesetTelescope* unit) {
    unit->m_fbufnum = -1e9f;
    unit->m_buf = nullptr;
    int startPos = (int)ZIN0(4);
    unit->m_srcPos = (startPos < 0) ? 0 : startPos;
    unit->m_count = 0;
    unit->m_refLen = 0;
    unit->m_phase = 0.0;
    unit->m_have = 0;
    SETCALC(WavesetTelescope_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetTelescope) {
    ft = inTable;
    DefineSimpleUnit(WavesetTelescope);
}
