#pragma once
#include "SC_PlugIn.h"

namespace waveset {

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

} // namespace waveset
