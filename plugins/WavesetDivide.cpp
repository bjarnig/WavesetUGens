// WavesetDivide - divides wavecycle frequency by `divider` (CDP DIVIDE), the
// dual of WavesetMultiply. Works on half-wavecycles: each group of `divider`
// half-cycles is replaced by the FIRST half-cycle stretched to fill the whole
// group's duration, with alternating polarity. Lowers pitch, preserves duration.
// Mono buffers only.

#include "waveset.hpp"

static InterfaceTable* ft;

struct WavesetDivide : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    int m_srcStart; // first half-cycle of the current group
    int m_srcLen;
    int m_totalLen; // total length of the `divider` half-cycles
    double m_ratio; // srcLen / totalLen
    double m_phase; // 0 .. m_totalLen
    float m_sign; // running output polarity (flips each group)
    int m_srcPos;
    int m_have;
};

void WavesetDivide_next(WavesetDivide* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    int divider = (int)ZIN0(1);
    float rate = ZIN0(2);
    if (divider < 1)
        divider = 1;
    if (divider > 16)
        divider = 16;
    if (rate <= 0.f)
        rate = 1.f;

    if (!bufData || bufChannels != 1 || bufFrames < 2) {
        ClearUnitOutputs(unit, inNumSamples);
        return;
    }

    int srcStart = unit->m_srcStart;
    int srcLen = unit->m_srcLen;
    int totalLen = unit->m_totalLen;
    double ratio = unit->m_ratio;
    double phase = unit->m_phase;
    float sign = unit->m_sign;
    int srcPos = unit->m_srcPos;
    int have = unit->m_have;
    const int frames = (int)bufFrames;

    for (int s = 0; s < inNumSamples; s++) {
        if (!have) {
            int p = (srcPos >= frames) ? 0 : srcPos;
            int e0 = waveset::findHalfEnd(bufData, frames, p);
            if (e0 < 0) {
                p = 0;
                e0 = waveset::findHalfEnd(bufData, frames, 0);
            }
            if (e0 < 0) {
                out[s] = 0.f;
                continue;
            }
            srcStart = p;
            srcLen = e0 - p;
            int total = srcLen;
            int pos = e0;
            for (int k = 1; k < divider; k++) { // accumulate the remaining half-cycles
                if (pos >= frames)
                    pos = 0;
                int e = waveset::findHalfEnd(bufData, frames, pos);
                if (e < 0)
                    break;
                total += (e - pos);
                pos = e;
            }
            totalLen = total;
            srcPos = pos;
            ratio = (double)srcLen / (double)totalLen;
            phase = 0.0;
            have = 1;
        }

        // the first half-cycle stretched across the whole group, alternating sign
        float v = waveset::readLin(bufData, frames, (double)srcStart + phase * ratio);
        out[s] = sign * ((v < 0.f) ? -v : v);

        phase += rate;
        if (phase >= (double)totalLen) {
            have = 0;
            sign = -sign;
        }
    }

    unit->m_srcStart = srcStart;
    unit->m_srcLen = srcLen;
    unit->m_totalLen = totalLen;
    unit->m_ratio = ratio;
    unit->m_phase = phase;
    unit->m_sign = sign;
    unit->m_srcPos = srcPos;
    unit->m_have = have;
}

void WavesetDivide_Ctor(WavesetDivide* unit) {
    unit->m_fbufnum = -1e9f;
    unit->m_buf = nullptr;
    int startPos = (int)ZIN0(3);
    unit->m_srcPos = (startPos < 0) ? 0 : startPos;
    unit->m_srcStart = 0;
    unit->m_srcLen = 0;
    unit->m_totalLen = 0;
    unit->m_ratio = 1.0;
    unit->m_phase = 0.0;
    unit->m_sign = 1.f;
    unit->m_have = 0;

    SETCALC(WavesetDivide_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetDivide) {
    ft = inTable;
    DefineSimpleUnit(WavesetDivide);
}
