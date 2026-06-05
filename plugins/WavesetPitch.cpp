// WavesetPitch - randomly transposes each wavecycle (CDP PITCH / "flexitone").
// The transposition wanders: a new random target in +/- `octvary` octaves is
// picked every random 1..`cyclecnt` cycles and interpolated cycle-by-cycle. Each
// cycle is resampled to its transposed length. Mono buffers only.

#include "waveset.hpp"
#include "SC_RGen.h"
#include <cmath>

static InterfaceTable* ft;

struct WavesetPitch : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    int m_cycStart; // current source cycle
    int m_origLen;
    int m_newLen; // transposed (resampled) length
    double m_phase; // 0 .. m_newLen
    int m_srcPos;
    int m_have;
    // wandering transposition state
    double m_last, m_next, m_step;
    int m_pos, m_groupLen, m_init;
    RGen m_rgen;
};

void WavesetPitch_next(WavesetPitch* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    float octvary = ZIN0(1);
    int cyclecnt = (int)ZIN0(2);
    float rate = ZIN0(4);
    if (octvary < 0.f)
        octvary = 0.f;
    if (cyclecnt < 1)
        cyclecnt = 1;
    if (rate <= 0.f)
        rate = 1.f;

    if (!bufData || bufChannels != 1 || bufFrames < 2) {
        ClearUnitOutputs(unit, inNumSamples);
        return;
    }

    int cycStart = unit->m_cycStart;
    int origLen = unit->m_origLen;
    int newLen = unit->m_newLen;
    double phase = unit->m_phase;
    int srcPos = unit->m_srcPos;
    int have = unit->m_have;
    const int frames = (int)bufFrames;

    for (int s = 0; s < inNumSamples; s++) {
        if (!have) {
            waveset::Span sp = waveset::nextWaveset(bufData, frames, srcPos, 1);
            if (sp.end < 0) {
                out[s] = 0.f;
                continue;
            }
            cycStart = sp.start;
            origLen = sp.end - sp.start;
            srcPos = sp.end;

            // advance the wandering transposition (one step per cycle)
            if (unit->m_init) {
                unit->m_groupLen = (int)(unit->m_rgen.frand() * (double)cyclecnt) + 1;
                unit->m_next = ((unit->m_rgen.frand() * 2.0) - 1.0) * (double)octvary;
                unit->m_step = (unit->m_next - unit->m_last) / (double)unit->m_groupLen;
                unit->m_pos = 0;
                unit->m_init = 0;
            } else if (++unit->m_pos < unit->m_groupLen) {
                unit->m_last += unit->m_step;
            } else {
                unit->m_groupLen = (int)(unit->m_rgen.frand() * (double)cyclecnt) + 1;
                unit->m_last = unit->m_next;
                unit->m_next = ((unit->m_rgen.frand() * 2.0) - 1.0) * (double)octvary;
                unit->m_step = (unit->m_next - unit->m_last) / (double)unit->m_groupLen;
                unit->m_pos = 0;
            }
            newLen = (int)(origLen * pow(2.0, unit->m_last) + 0.5);
            if (newLen < 1)
                newLen = 1;
            phase = 0.0;
            have = 1;
        }

        double ratio = phase / (double)newLen;
        out[s] = waveset::readLin(bufData, frames, (double)cycStart + ratio * (double)origLen);

        phase += rate;
        if (phase >= (double)newLen)
            have = 0;
    }

    unit->m_cycStart = cycStart;
    unit->m_origLen = origLen;
    unit->m_newLen = newLen;
    unit->m_phase = phase;
    unit->m_srcPos = srcPos;
    unit->m_have = have;
}

void WavesetPitch_Ctor(WavesetPitch* unit) {
    unit->m_fbufnum = -1e9f;
    unit->m_buf = nullptr;
    int startPos = (int)ZIN0(5);
    unit->m_srcPos = (startPos < 0) ? 0 : startPos;
    unit->m_cycStart = 0;
    unit->m_origLen = 0;
    unit->m_newLen = 0;
    unit->m_phase = 0.0;
    unit->m_have = 0;
    unit->m_last = 0.0;
    unit->m_next = 0.0;
    unit->m_step = 0.0;
    unit->m_pos = 0;
    unit->m_groupLen = 1;
    unit->m_init = 1;
    unit->m_rgen.init((uint32)(int)ZIN0(3)); // seed

    SETCALC(WavesetPitch_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetPitch) {
    ft = inTable;
    DefineSimpleUnit(WavesetPitch);
}
