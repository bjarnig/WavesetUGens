// === WavesetReplace tests ===
// The loudest wavecycle of each group of `cyclecnt` (by sum of |samples|)
// replaces the others — the group becomes that cycle repeated cyclecnt times.
s.boot;
b = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav"); // mono

// dry reference
( x = { PlayBuf.ar(1, b, BufRateScale.kr(b), loop: 1) * 0.5 ! 2 }.play; )
x.free;

// 1) groups of 2 — mild simplification
( x = { WavesetReplace.ar(b, 2) * 0.5 ! 2 }.play; )
x.free;

// 2) groups of 4 — coarser, more "locked" timbre
( x = { WavesetReplace.ar(b, 4) * 0.5 ! 2 }.play; )
x.free;

// 3) groups of 12 — strong, buzzy plateaus
( x = { WavesetReplace.ar(b, 12) * 0.5 ! 2 }.play; )
x.free;

// 4) replace + transpose down
( x = { WavesetReplace.ar(b, 6, rate: 0.5) * 0.5 ! 2 }.play; )
x.free;

// 5) modulate the group size
(
x = { WavesetReplace.ar(b, LFNoise0.kr(2).range(2, 16).round) * 0.5 ! 2 }.play;
)
x.free;
