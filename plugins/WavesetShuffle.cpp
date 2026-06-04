// WavesetShuffle - CDP SHUFFLE. A window of `domain` consecutive wavesets is
// emitted in the order given by `image`, a sequence of indices into the domain
// that may reorder, omit, or duplicate them (duplication time-stretches). Each
// waveset is `cyclecnt` wavecycles. Mono buffers only.
//
// Inputs: bufnum, cyclecnt, rate, startPos, domainSize, imageLen, image[0..].

#include "waveset.hpp"

static const int kImageBase = 6; // first image-index input

static InterfaceTable* ft;

struct WavesetShuffle : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    int m_start[waveset::kMaxGroup];
    int m_len[waveset::kMaxGroup];
    int m_count; // wavesets detected in the current domain window
    int m_imgPos; // >= imageLen => load the next window
    int m_srcPos;
    int m_curStart;
    int m_curLen;
    double m_phase;
};

void WavesetShuffle_next(WavesetShuffle* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    int cyclecnt = (int)ZIN0(1);
    float rate = ZIN0(2);
    int domainSize = (int)ZIN0(4);
    int imageLen = (int)ZIN0(5);
    if (cyclecnt < 1)
        cyclecnt = 1;
    if (domainSize < 1)
        domainSize = 1;
    if (domainSize > waveset::kMaxGroup)
        domainSize = waveset::kMaxGroup;
    if (imageLen < 1)
        imageLen = 1;
    if (rate <= 0.f)
        rate = 1.f;

    if (!bufData || bufChannels != 1 || bufFrames < 2) {
        ClearUnitOutputs(unit, inNumSamples);
        return;
    }

    int count = unit->m_count;
    int imgPos = unit->m_imgPos;
    int srcPos = unit->m_srcPos;
    int curStart = unit->m_curStart;
    int curLen = unit->m_curLen;
    double phase = unit->m_phase;
    const int frames = (int)bufFrames;

    for (int s = 0; s < inNumSamples; s++) {
        if (curLen <= 0) {
            if (imgPos >= imageLen) {
                int n = 0;
                int pos = srcPos;
                for (; n < domainSize; n++) {
                    waveset::Span sp = waveset::nextWaveset(bufData, frames, pos, cyclecnt);
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
                imgPos = 0;
            }
            int idx = (int)ZIN0(kImageBase + imgPos);
            if (idx < 0)
                idx = 0;
            if (idx >= count)
                idx = count - 1;
            curStart = unit->m_start[idx];
            curLen = unit->m_len[idx];
            phase = 0.0;
        }

        out[s] = waveset::readLin(bufData, frames, (double)curStart + phase);

        phase += rate;
        if (phase >= (double)curLen) {
            imgPos++;
            curLen = 0;
        }
    }

    unit->m_count = count;
    unit->m_imgPos = imgPos;
    unit->m_srcPos = srcPos;
    unit->m_curStart = curStart;
    unit->m_curLen = curLen;
    unit->m_phase = phase;
}

void WavesetShuffle_Ctor(WavesetShuffle* unit) {
    unit->m_fbufnum = -1e9f;
    unit->m_buf = nullptr;

    int startPos = (int)ZIN0(3);
    unit->m_srcPos = (startPos < 0) ? 0 : startPos;
    unit->m_count = 0;
    unit->m_imgPos = 1 << 30; // force the first block to load a window
    unit->m_curStart = 0;
    unit->m_curLen = 0;
    unit->m_phase = 0.0;

    SETCALC(WavesetShuffle_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetShuffle) {
    ft = inTable;
    DefineSimpleUnit(WavesetShuffle);
}
