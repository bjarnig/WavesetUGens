// WavesetRepeat - a buffer-reading "waveset distortion" UGen.
//
// Port of the CDP (Composers Desktop Project) waveset *repeat* process to a
// SuperCollider buffer UGen, in the spirit of GrainBuf: the source sound lives
// in a Buffer (random access, in RAM) and a read-pointer walks it, so the UGen
// can emit MORE samples than it reads -- which is exactly what waveset repeat
// needs. Each "waveset" is `numCycles` full wavecycles delimited by zero
// crossings (cf. CDP get_full_cycle in dev/distort/distorte.c); each detected
// waveset is replayed `repeats` times before the pointer advances.
//
// Mono buffers only (waveset processes are classically mono).
//
// Anti-aliasing: repeating wavesets introduces slope discontinuities at the
// seams whose harmonics fold back as aliasing. The optional `oversample` factor
// runs the read/repeat loop at M x the sample rate and decimates through an
// 8th-order Butterworth low-pass, removing content above Nyquist before it
// folds. oversample == 1 bypasses the filter (identical to the un-oversampled
// algorithm).

#include "SC_PlugIn.h"
#include <cmath>

static InterfaceTable* ft;

// Section Qs for an 8th-order Butterworth, split into 4 biquads:
// Q_k = 1 / (2 cos(pi (2k+1) / 16)).
static const double kButterQ8[4] = { 0.50979558, 0.60134489, 0.89997622, 2.56291545 };

struct WavesetRepeat : public Unit {
    // --- required by the GET_BUF macro ---
    float m_fbufnum;
    SndBuf* m_buf;

    // --- waveset playback state, persisted across control blocks ---
    double m_readPos; // frame index in buffer: start of the CURRENT waveset
    double m_phase; // playback offset within the current waveset, in frames
    int m_wavesetLen; // length of current waveset in frames (0 => none loaded)
    int m_repeatsLeft; // repeats remaining for the current waveset

    // --- anti-aliasing decimation filter (only used when m_os > 1) ---
    int m_os; // oversampling factor: 1 (off), 2, 4, or 8
    double m_b0[4], m_b1[4], m_b2[4], m_a1[4], m_a2[4]; // normalized biquad coeffs
    double m_z1[4], m_z2[4]; // per-biquad state
};

// Scan forward from `start`, covering `numCycles` full wavecycles, and return
// the frame index one past the end of that waveset. Mirrors CDP's
// get_full_cycle(): a full cycle = run through the current sign region, then
// the opposite sign region, back to the starting sign. Returns -1 if the scan
// runs past the end of the buffer (caller decides whether to wrap).
//
// Called at most once per emitted waveset (not per sample). Worst case for a
// pathological (e.g. DC) buffer is one full scan of `frames`, which is bounded.
static inline int findWavesetEnd(const float* data, int frames, int start, int numCycles) {
    int i = start;
    for (int n = 0; n < numCycles; n++) {
        if (data[i] >= 0.f) {
            while (i < frames && data[i] >= 0.f)
                i++;
            while (i < frames && data[i] <= 0.f)
                i++;
        } else {
            while (i < frames && data[i] <= 0.f)
                i++;
            while (i < frames && data[i] >= 0.f)
                i++;
        }
        if (i >= frames)
            return -1;
    }
    return i;
}

// One sample through the 4-biquad Butterworth cascade (transposed direct form II).
static inline double aaProcess(WavesetRepeat* unit, double x) {
    for (int i = 0; i < 4; i++) {
        double y = unit->m_b0[i] * x + unit->m_z1[i];
        unit->m_z1[i] = unit->m_b1[i] * x - unit->m_a1[i] * y + unit->m_z2[i];
        unit->m_z2[i] = unit->m_b2[i] * x - unit->m_a2[i] * y;
        x = y;
    }
    return x;
}

void WavesetRepeat_next(WavesetRepeat* unit, int inNumSamples) {
    GET_BUF // -> buf, bufData, bufChannels, bufFrames, ... (locks the buffer)

    float* out = OUT(0);

    // Input order must match the .sc class file:
    // 0 = bufnum (consumed by GET_BUF), 1 = repeats, 2 = rate, 3 = numCycles,
    // 4 = startPos, 5 = oversample (both read once, in the Ctor).
    int repeats = (int)ZIN0(1);
    float rate = ZIN0(2);
    int numCycles = (int)ZIN0(3);
    if (repeats < 1)
        repeats = 1;
    if (numCycles < 1)
        numCycles = 1;
    if (rate <= 0.f)
        rate = 1.f; // negative/zero rate unsupported in this pilot

    // Bail to silence on a missing or non-mono buffer.
    if (!bufData || bufChannels != 1 || bufFrames < 2) {
        ClearUnitOutputs(unit, inNumSamples);
        return;
    }

    double readPos = unit->m_readPos;
    double phase = unit->m_phase;
    int wavesetLen = unit->m_wavesetLen;
    int repeatsLeft = unit->m_repeatsLeft;
    const int frames = (int)bufFrames;

    const int M = unit->m_os; // oversampling factor (>= 1)
    const double rateStep = (double)rate / (double)M; // per-oversampled-step advance

    for (int s = 0; s < inNumSamples; s++) {
        double outSample = 0.0;

        // Compute M oversampled sub-samples; the decimated output is the last one
        // after low-pass filtering. With M == 1 the filter is bypassed.
        for (int m = 0; m < M; m++) {
            double raw = 0.0;

            // (Re)load a waveset whenever we don't currently have one.
            if (wavesetLen <= 0) {
                if ((int)readPos >= frames)
                    readPos = 0.0; // loop the source
                int end = findWavesetEnd(bufData, frames, (int)readPos, numCycles);
                if (end < 0) {
                    // Tail too short to contain a whole waveset -> wrap to start.
                    readPos = 0.0;
                    end = findWavesetEnd(bufData, frames, 0, numCycles);
                }
                if (end < 0) {
                    // Buffer can't hold even one waveset: emit silence (still filter,
                    // so the cascade settles cleanly).
                    outSample = (M == 1) ? 0.0 : aaProcess(unit, 0.0);
                    continue;
                }
                wavesetLen = end - (int)readPos;
                repeatsLeft = repeats;
                phase = 0.0;
            }

            // Read current waveset sample with linear interpolation (rate transpose).
            double rp = readPos + phase;
            int i0 = (int)rp;
            double frac = rp - (double)i0;
            int i1 = i0 + 1;
            float a = bufData[i0];
            float b = (i1 < frames) ? bufData[i1] : bufData[i0];
            raw = (double)a + frac * ((double)b - (double)a);

            // Advance within the waveset; loop or move on at its end.
            phase += rateStep;
            if (phase >= (double)wavesetLen) {
                repeatsLeft--;
                if (repeatsLeft > 0) {
                    phase -= (double)wavesetLen; // replay the same waveset
                } else {
                    readPos += (double)wavesetLen; // advance to the next waveset
                    wavesetLen = 0; // force a reload next step
                }
            }

            outSample = (M == 1) ? raw : aaProcess(unit, raw);
        }

        out[s] = (float)outSample;
    }

    unit->m_readPos = readPos;
    unit->m_phase = phase;
    unit->m_wavesetLen = wavesetLen;
    unit->m_repeatsLeft = repeatsLeft;
}

void WavesetRepeat_Ctor(WavesetRepeat* unit) {
    unit->m_fbufnum = -1e9f; // force GET_BUF to (re)resolve the buffer
    unit->m_buf = nullptr;

    int startPos = (int)ZIN0(4);
    if (startPos < 0)
        startPos = 0;
    unit->m_readPos = (double)startPos;
    unit->m_phase = 0.0;
    unit->m_wavesetLen = 0; // first sample will detect the first waveset
    unit->m_repeatsLeft = 0;

    // Oversampling factor: snap to {1, 2, 4, 8}, read once at init time.
    int os = (int)ZIN0(5);
    if (os < 2)
        os = 1;
    else if (os < 4)
        os = 2;
    else if (os < 8)
        os = 4;
    else
        os = 8;
    unit->m_os = os;

    // Design the decimation low-pass (8th-order Butterworth) at the oversampled
    // rate, cutoff safely below the original Nyquist.
    for (int i = 0; i < 4; i++) {
        unit->m_z1[i] = 0.0;
        unit->m_z2[i] = 0.0;
    }
    if (os > 1) {
        const double fs = (double)SAMPLERATE * (double)os;
        const double fc = 0.45 * (double)SAMPLERATE;
        const double w0 = 2.0 * M_PI * fc / fs;
        const double cw = cos(w0);
        const double sw = sin(w0);
        for (int i = 0; i < 4; i++) {
            const double alpha = sw / (2.0 * kButterQ8[i]);
            const double a0 = 1.0 + alpha;
            const double b0 = (1.0 - cw) * 0.5;
            const double b1 = (1.0 - cw);
            const double b2 = (1.0 - cw) * 0.5;
            const double a1 = -2.0 * cw;
            const double a2 = 1.0 - alpha;
            unit->m_b0[i] = b0 / a0;
            unit->m_b1[i] = b1 / a0;
            unit->m_b2[i] = b2 / a0;
            unit->m_a1[i] = a1 / a0;
            unit->m_a2[i] = a2 / a0;
        }
    }

    SETCALC(WavesetRepeat_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetRepeat) {
    ft = inTable;
    DefineSimpleUnit(WavesetRepeat);
}
