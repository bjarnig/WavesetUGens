// WavesetHarmonic - adds harmonics over each wavecycle (CDP HARMONIC). For each
// harmonic number h with amplitude a, a copy of the cycle read at h x speed
// (wrapped) is scaled by a and summed onto the cycle. Harmonics at or above
// len/4 are skipped to limit foldover. Mono buffers only.
//
// Inputs: bufnum, rate, startPos, hcnt, hno[0..], amp[0..].

#include "waveset.hpp"
#include <cmath>

static const int kHarmBase = 4;

static InterfaceTable* ft;

struct WavesetHarmonic : public Unit {
    float m_fbufnum; // required by GET_BUF
    SndBuf* m_buf;
    double m_readPos; // start of the current cycle
    double m_phase;
    int m_cycLen;
};

void WavesetHarmonic_next(WavesetHarmonic* unit, int inNumSamples) {
    GET_BUF
    float* out = OUT(0);

    float rate = ZIN0(1);
    int hcnt = (int)ZIN0(3);
    if (rate <= 0.f)
        rate = 1.f;
    if (hcnt < 0)
        hcnt = 0;

    if (!bufData || bufChannels != 1 || bufFrames < 2) {
        ClearUnitOutputs(unit, inNumSamples);
        return;
    }

    double readPos = unit->m_readPos;
    double phase = unit->m_phase;
    int cycLen = unit->m_cycLen;
    const int frames = (int)bufFrames;

    for (int s = 0; s < inNumSamples; s++) {
        if (cycLen <= 0) {
            waveset::Span sp = waveset::nextWaveset(bufData, frames, (int)readPos, 1);
            if (sp.end < 0) {
                out[s] = 0.f;
                continue;
            }
            readPos = (double)sp.start;
            cycLen = sp.end - sp.start;
            phase = 0.0;
        }

        double acc = waveset::readLin(bufData, frames, readPos + phase); // original cycle
        int foldLimit = cycLen / 4;
        for (int n = 0; n < hcnt; n++) {
            int h = (int)ZIN0(kHarmBase + n);
            if (h < 1 || h >= foldLimit)
                continue;
            float a = ZIN0(kHarmBase + hcnt + n);
            double hp = fmod(phase * (double)h, (double)cycLen);
            acc += (double)a * waveset::readLin(bufData, frames, readPos + hp);
        }
        out[s] = (float)acc;

        phase += rate;
        if (phase >= (double)cycLen) {
            readPos += (double)cycLen;
            cycLen = 0;
        }
    }

    unit->m_readPos = readPos;
    unit->m_phase = phase;
    unit->m_cycLen = cycLen;
}

void WavesetHarmonic_Ctor(WavesetHarmonic* unit) {
    unit->m_fbufnum = -1e9f;
    unit->m_buf = nullptr;
    int startPos = (int)ZIN0(2);
    unit->m_readPos = (startPos < 0) ? 0.0 : (double)startPos;
    unit->m_phase = 0.0;
    unit->m_cycLen = 0;
    SETCALC(WavesetHarmonic_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetHarmonic) {
    ft = inTable;
    DefineSimpleUnit(WavesetHarmonic);
}
