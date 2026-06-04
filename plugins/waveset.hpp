#pragma once
#include "SC_PlugIn.h"

namespace waveset {

static const double kTwoPi = 6.283185307179586; // portable (MSVC lacks M_PI)
static const int kMaxGroup = 256; // cap on wavesets buffered per group/window

// Frame index one past `numCycles` full wavecycles from `start`, or -1 if the
// scan runs off the end. Called once per emitted waveset, not per sample.
static inline int findEnd(const float* data, int frames, int start, int numCycles) {
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

// Linear-interpolated read with clamped right neighbor.
static inline float readLin(const float* data, int frames, double pos) {
    int i0 = (int)pos;
    double frac = pos - (double)i0;
    float a = data[i0];
    float b = (i0 + 1 < frames) ? data[i0 + 1] : data[i0];
    return (float)(a + frac * (b - a));
}

// Half-open span of a detected waveset; end < 0 means none could be found.
struct Span {
    int start;
    int end;
};

// Next waveset at or after `pos`, wrapping to 0 at the end of the buffer.
static inline Span nextWaveset(const float* data, int frames, int pos, int numCycles) {
    if (pos >= frames)
        pos = 0;
    int end = findEnd(data, frames, pos, numCycles);
    if (end < 0) {
        pos = 0;
        end = findEnd(data, frames, 0, numCycles);
    }
    return Span { pos, end };
}

// Peak absolute value over [start, start + len).
static inline float peakAbs(const float* data, int start, int len) {
    float p = 0.f;
    for (int i = 0; i < len; i++) {
        float a = data[start + i];
        if (a < 0.f)
            a = -a;
        if (a > p)
            p = a;
    }
    return p;
}

} // namespace waveset
