// WavesetInterpolate - morphs between consecutive wavesets, emitting `multiplier`
// crossfaded wavesets per source waveset (a waveset-based time-stretch). Mono.

#include "waveset.hpp"

static InterfaceTable* ft;

struct WavesetInterpolate : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    int m_startA, m_lenA, m_startB, m_lenB;
    int m_srcPos;
    int m_k; // morph step within the current pair
    double m_f; // current crossfade amount
    int m_outLen; // length of the current output waveset
    double m_phase;
    int m_inited;
};

void WavesetInterpolate_next(WavesetInterpolate* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    int multiplier = (int)ZIN0(1);
    float rate = ZIN0(2);
    int cyclecnt = (int)ZIN0(3);
    if (multiplier < 1)
        multiplier = 1;
    if (cyclecnt < 1)
        cyclecnt = 1;
    if (rate <= 0.f)
        rate = 1.f;

    if (!bufData || bufChannels != 1 || bufFrames < 2) {
        ClearUnitOutputs(unit, inNumSamples);
        return;
    }

    int startA = unit->m_startA, lenA = unit->m_lenA;
    int startB = unit->m_startB, lenB = unit->m_lenB;
    int srcPos = unit->m_srcPos;
    int k = unit->m_k;
    double f = unit->m_f;
    int outLen = unit->m_outLen;
    double phase = unit->m_phase;
    int inited = unit->m_inited;
    const int frames = (int)bufFrames;

    for (int s = 0; s < inNumSamples; s++) {
        if (!inited) {
            waveset::Span a = waveset::nextWaveset(bufData, frames, srcPos, cyclecnt);
            if (a.end < 0) {
                out[s] = 0.f;
                continue;
            }
            startA = a.start;
            lenA = a.end - a.start;
            waveset::Span b = waveset::nextWaveset(bufData, frames, a.end, cyclecnt);
            startB = (b.end < 0) ? startA : b.start;
            lenB = (b.end < 0) ? lenA : (b.end - b.start);
            srcPos = (b.end < 0) ? a.end : b.end;
            k = 0;
            outLen = 0;
            inited = 1;
        }

        if (outLen <= 0) {
            f = (double)k / (double)multiplier;
            outLen = (int)((1.0 - f) * lenA + f * lenB + 0.5);
            if (outLen < 1)
                outLen = 1;
            phase = 0.0;
        }

        double p = phase / (double)outLen; // 0..1
        float a = waveset::readLin(bufData, frames, (double)startA + p * (double)lenA);
        float b = waveset::readLin(bufData, frames, (double)startB + p * (double)lenB);
        out[s] = (float)((1.0 - f) * a + f * b);

        phase += rate;
        if (phase >= (double)outLen) {
            outLen = 0;
            k++;
            if (k >= multiplier) { // advance the pair: A <- B, B <- next
                k = 0;
                startA = startB;
                lenA = lenB;
                waveset::Span nb = waveset::nextWaveset(bufData, frames, srcPos, cyclecnt);
                if (nb.end >= 0) {
                    startB = nb.start;
                    lenB = nb.end - nb.start;
                    srcPos = nb.end;
                }
            }
        }
    }

    unit->m_startA = startA;
    unit->m_lenA = lenA;
    unit->m_startB = startB;
    unit->m_lenB = lenB;
    unit->m_srcPos = srcPos;
    unit->m_k = k;
    unit->m_f = f;
    unit->m_outLen = outLen;
    unit->m_phase = phase;
    unit->m_inited = inited;
}

void WavesetInterpolate_Ctor(WavesetInterpolate* unit) {
    unit->m_fbufnum = -1e9f;
    unit->m_buf = nullptr;

    int startPos = (int)ZIN0(4);
    unit->m_srcPos = (startPos < 0) ? 0 : startPos;
    unit->m_startA = unit->m_lenA = 0;
    unit->m_startB = unit->m_lenB = 0;
    unit->m_k = 0;
    unit->m_f = 0.0;
    unit->m_outLen = 0;
    unit->m_phase = 0.0;
    unit->m_inited = 0;

    SETCALC(WavesetInterpolate_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetInterpolate) {
    ft = inTable;
    DefineSimpleUnit(WavesetInterpolate);
}
