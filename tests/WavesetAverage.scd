// === WavesetAverage tests ===
// Averages a group of `cyclecnt` wavecycles into one mean-length cycle — a
// loss-of-resolution / time-contraction effect (like Telescope, mean length).
s.boot;
b = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav"); // mono

// dry reference
( x = { PlayBuf.ar(1, b, BufRateScale.kr(b), loop: 1) * 0.5 ! 2 }.play; )
x.free;

// 1) groups of 2
( x = { WavesetAverage.ar(b, 2) * 0.6 ! 2 }.play; )
x.free;

// 2) groups of 8 — grainier, faster
( x = { WavesetAverage.ar(b, 8) * 0.6 ! 2 }.play; )
x.free;

// 3) groups of 32 — heavy smear / pitch-down feel
( x = { WavesetAverage.ar(b, 32) * 0.7 ! 2 }.play; )
x.free;

// 4) average + transpose up to offset the contraction
( x = { WavesetAverage.ar(b, 8, rate: 1.5) * 0.55 ! 2 }.play; )
x.free;

// 5) modulate the group size
(
x = { WavesetAverage.ar(b, LFNoise0.kr(2).range(2, 24).round) * 0.6 ! 2 }.play;
)
x.free;
