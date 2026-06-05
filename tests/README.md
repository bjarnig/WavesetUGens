# WavesetUGens tests

Interactive SuperCollider test scripts — one `.scd` per UGen. They're for
listening, not automated assertions: evaluate blocks one at a time in the IDE.

## How to use
1. Make sure the plugins are installed and the class library recompiled
   (Cmd/Ctrl-Shift-L), then boot the server.
2. Open a `.scd` file. Each starts with a setup line (boot + load a **mono**
   source buffer) and a dry reference for A/B comparison.
3. Put the cursor inside a `( ... )` block and press Cmd/Ctrl-Enter to play it.
   Evaluate the following `x.free;` to stop before trying the next one.

All examples use the bundled mono sound `a11wlk01.wav`. Swap in your own mono
file by changing the `Buffer.read` path — waveset processes are mono-only and
most characterful on quasi-periodic material (voice, sustained tones, drones).

Tip: the effect of `cyclecnt` (wavecycles grouped per waveset) grows with larger
values — single cycles are subtle, groups of 8–40 are dramatic.
