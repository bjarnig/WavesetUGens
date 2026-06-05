// WavesetMultiply - multiplies wavecycle frequency by `multiplier` (CDP
// MULTIPLY). Works on single half-wavecycles: each is compressed to 1/multiplier
// of its length and emitted `multiplier` times with alternating polarity, so the
// span fills with `multiplier` half-cycles (pitch up, duration preserved). Mono.

#include "waveset.hpp"

static InterfaceTable* ft;

struct WavesetMultiply : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    int m_halfStart; // current source half-cycle
    int m_halfLen;
    double m_subLen; // length of one compressed copy (0 => (re)compute)
    int m_copy; // copy index within the half-cycle
    double m_phase;
    float m_sign; // running output polarity (flips each emitted copy)
    int m_srcPos;
};

void WavesetMultiply_next(WavesetMultiply* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    int multiplier = (int)ZIN0(1);
    float rate = ZIN0(2);
    if (multiplier < 1)
        multiplier = 1;
    if (multiplier > 16)
        multiplier = 16;
    if (rate <= 0.f)
        rate = 1.f;

    if (!bufData || bufChannels != 1 || bufFrames < 2) {
        ClearUnitOutputs(unit, inNumSamples);
        return;
    }

    int halfStart = unit->m_halfStart;
    int halfLen = unit->m_halfLen;
    double subLen = unit->m_subLen;
    int copy = unit->m_copy;
    double phase = unit->m_phase;
    float sign = unit->m_sign;
    int srcPos = unit->m_srcPos;
    const int frames = (int)bufFrames;

    for (int s = 0; s < inNumSamples; s++) {
        if (subLen <= 0.0) {
            if (copy >= multiplier || halfLen <= 0) {
                int p = (srcPos >= frames) ? 0 : srcPos;
                int e = waveset::findHalfEnd(bufData, frames, p);
                if (e < 0) {
                    p = 0;
                    e = waveset::findHalfEnd(bufData, frames, 0);
                }
                if (e < 0) {
                    out[s] = 0.f;
                    continue;
                }
                halfStart = p;
                halfLen = e - p;
                srcPos = e;
                copy = 0;
            }
            subLen = (double)halfLen / (double)multiplier;
            if (subLen < 1.0)
                subLen = (double)halfLen; // half shorter than multiplier: one copy
            phase = 0.0;
        }

        // read the half-cycle compressed into subLen, take its magnitude, and
        // apply the running polarity so successive copies alternate sign.
        double rp = (double)halfStart + phase * (double)multiplier;
        float v = waveset::readLin(bufData, frames, rp);
        out[s] = sign * ((v < 0.f) ? -v : v);

        phase += rate;
        if (phase >= subLen) {
            phase -= subLen;
            copy++;
            sign = -sign;
            subLen = 0.0; // recompute (next copy, or detect a new half-cycle)
        }
    }

    unit->m_halfStart = halfStart;
    unit->m_halfLen = halfLen;
    unit->m_subLen = subLen;
    unit->m_copy = copy;
    unit->m_phase = phase;
    unit->m_sign = sign;
    unit->m_srcPos = srcPos;
}

void WavesetMultiply_Ctor(WavesetMultiply* unit) {
    unit->m_fbufnum = -1e9f;
    unit->m_buf = nullptr;

    int startPos = (int)ZIN0(3);
    unit->m_srcPos = (startPos < 0) ? 0 : startPos;
    unit->m_halfStart = 0;
    unit->m_halfLen = 0;
    unit->m_subLen = 0.0;
    unit->m_copy = 1 << 30; // force first block to detect a half-cycle
    unit->m_phase = 0.0;
    unit->m_sign = 1.f;

    SETCALC(WavesetMultiply_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetMultiply) {
    ft = inTable;
    DefineSimpleUnit(WavesetMultiply);
}
