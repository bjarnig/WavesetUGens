// === WavesetTelescope tests ===
// Superimposes (averages) a group of `cyclecnt` wavecycles into one, contracting
// time. Output cycle length = longest of the group (or mean if useMean = 1).
s.boot;
b = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav"); // mono

// dry reference
( x = { PlayBuf.ar(1, b, BufRateScale.kr(b), loop: 1) * 0.5 ! 2 }.play; )
x.free;

// 1) groups of 2 — slight contraction + blending
( x = { WavesetTelescope.ar(b, 2) * 0.5 ! 2 }.play; )
x.free;

// 2) groups of 8 — faster, smeared
( x = { WavesetTelescope.ar(b, 8) * 0.6 ! 2 }.play; )
x.free;

// 3) groups of 24 — strong time-compression
( x = { WavesetTelescope.ar(b, 24) * 0.7 ! 2 }.play; )
x.free;

// 4) mean-length output (useMean: 1) vs longest — compare with #2
( x = { WavesetTelescope.ar(b, 8, useMean: 1) * 0.6 ! 2 }.play; )
x.free;

// 5) modulate the group size
(
x = { WavesetTelescope.ar(b, LFNoise0.kr(2).range(2, 20).round) * 0.6 ! 2 }.play;
)
x.free;
