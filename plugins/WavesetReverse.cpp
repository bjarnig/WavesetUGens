// WavesetReverse - plays each group of `cyclecnt` wavecycles backwards (CDP
// REVERSE reverses the samples of the whole group: 3-2-1, 6-5-4, ...). Mono.

#include "waveset.hpp"

static InterfaceTable* ft;

struct WavesetReverse : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    double m_readPos; // start of the current group, in frames
    double m_phase;
    int m_groupLen; // length of the current group in frames (0 => none loaded)
};

void WavesetReverse_next(WavesetReverse* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    int cyclecnt = (int)ZIN0(1);
    float rate = ZIN0(2);
    if (cyclecnt < 1)
        cyclecnt = 1;
    if (rate <= 0.f)
        rate = 1.f;

    if (!bufData || bufChannels != 1 || bufFrames < 2) {
        ClearUnitOutputs(unit, inNumSamples);
        return;
    }

    double readPos = unit->m_readPos;
    double phase = unit->m_phase;
    int groupLen = unit->m_groupLen;
    const int frames = (int)bufFrames;

    for (int s = 0; s < inNumSamples; s++) {
        if (groupLen <= 0) {
            waveset::Span sp = waveset::nextWaveset(bufData, frames, (int)readPos, cyclecnt);
            if (sp.end < 0) {
                out[s] = 0.f;
                continue;
            }
            readPos = (double)sp.start;
            groupLen = sp.end - sp.start;
            phase = 0.0;
        }

        // read the group from its end backwards (sample reversal)
        out[s] = waveset::readLin(bufData, frames, readPos + ((double)groupLen - phase));

        phase += rate;
        if (phase >= (double)groupLen) {
            readPos += (double)groupLen;
            groupLen = 0;
        }
    }

    unit->m_readPos = readPos;
    unit->m_phase = phase;
    unit->m_groupLen = groupLen;
}

void WavesetReverse_Ctor(WavesetReverse* unit) {
    unit->m_fbufnum = -1e9f;
    unit->m_buf = nullptr;

    int startPos = (int)ZIN0(3);
    unit->m_readPos = (startPos < 0) ? 0.0 : (double)startPos;
    unit->m_phase = 0.0;
    unit->m_groupLen = 0;

    SETCALC(WavesetReverse_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetReverse) {
    ft = inTable;
    DefineSimpleUnit(WavesetReverse);
}
