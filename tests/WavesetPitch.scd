// === WavesetPitch tests ===
// Randomly transposes each wavecycle; the transposition wanders, picking a new
// target in +/- `octvary` octaves every random 1..`cyclecnt` cycles. `seed`
// makes a given wander reproducible.
s.boot;
b = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav"); // mono

// dry reference
( x = { PlayBuf.ar(1, b, BufRateScale.kr(b), loop: 1) * 0.5 ! 2 }.play; )
x.free;

// 1) gentle shimmer
( x = { WavesetPitch.ar(b, octvary: 0.15, cyclecnt: 64, seed: 1) * 0.5 ! 2 }.play; )
x.free;

// 2) wider, faster bending (shorter cyclecnt = quicker changes)
( x = { WavesetPitch.ar(b, octvary: 0.5, cyclecnt: 16, seed: 7) * 0.5 ! 2 }.play; )
x.free;

// 3) wild flexitone
( x = { WavesetPitch.ar(b, octvary: 1.0, cyclecnt: 8, seed: 42) * 0.5 ! 2 }.play; )
x.free;

// 4) same settings, different seeds — compare the two wanders
( x = { WavesetPitch.ar(b, 0.5, 24, seed: 3) * 0.5 ! 2 }.play; )
x.free;
( x = { WavesetPitch.ar(b, 0.5, 24, seed: 99) * 0.5 ! 2 }.play; )
x.free;

// 5) stereo decorrelation: two seeds panned L/R
( x = { [WavesetPitch.ar(b, 0.4, 20, 1), WavesetPitch.ar(b, 0.4, 20, 2)] * 0.5 }.play; )
x.free;
