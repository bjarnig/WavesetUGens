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

#include "SC_PlugIn.h"

static InterfaceTable* ft;

struct WavesetRepeat : public Unit {
    // --- required by the GET_BUF macro ---
    float m_fbufnum;
    SndBuf* m_buf;

    // --- waveset playback state, persisted across control blocks ---
    double m_readPos; // frame index in buffer: start of the CURRENT waveset
    double m_phase; // playback offset within the current waveset, in frames
    int m_wavesetLen; // length of current waveset in frames (0 => none loaded)
    int m_repeatsLeft; // repeats remaining for the current waveset
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

void WavesetRepeat_next(WavesetRepeat* unit, int inNumSamples) {
    GET_BUF // -> buf, bufData, bufChannels, bufFrames, ... (locks the buffer)

    float* out = OUT(0);

    // Input order must match the .sc class file:
    // 0 = bufnum (consumed by GET_BUF), 1 = repeats, 2 = rate,
    // 3 = numCycles, 4 = startPos (read once, in the Ctor).
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

    for (int s = 0; s < inNumSamples; s++) {
        // (Re)load a waveset whenever we don't currently have one.
        if (wavesetLen <= 0) {
            if ((int)readPos >= frames)
                readPos = 0.0; // loop the source
            int end = findWavesetEnd(bufData, frames, (int)readPos, numCycles);
            if (end < 0) {
                // Tail too short to contain a whole waveset -> wrap to start.
                readPos = 0.0;
                end = findWavesetEnd(bufData, frames, 0, numCycles);
                if (end < 0) {
                    out[s] = 0.f; // buffer can't hold even one waveset
                    continue;
                }
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
        out[s] = (float)(a + frac * (b - a));

        // Advance within the waveset; loop or move on at its end.
        phase += rate;
        if (phase >= (double)wavesetLen) {
            repeatsLeft--;
            if (repeatsLeft > 0) {
                phase -= (double)wavesetLen; // replay the same waveset
            } else {
                readPos += (double)wavesetLen; // advance to the next waveset
                wavesetLen = 0; // force a reload next sample
            }
        }
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

    SETCALC(WavesetRepeat_next);
    ClearUnitOutputs(unit, 1);
}

PluginLoad(WavesetRepeat) {
    ft = inTable;
    DefineSimpleUnit(WavesetRepeat);
}
