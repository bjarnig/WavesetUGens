// === WavesetMultiply tests ===
// Multiplies wavecycle frequency by `multiplier` (pitch up, duration preserved).
s.boot;
b = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav"); // mono

// dry reference
( x = { PlayBuf.ar(1, b, BufRateScale.kr(b), loop: 1) * 0.5 ! 2 }.play; )
x.free;

// 1) x2 — up an octave in frequency, same duration
( x = { WavesetMultiply.ar(b, 2) * 0.4 ! 2 }.play; )
x.free;

// 2) x3 — brighter, buzzier
( x = { WavesetMultiply.ar(b, 3) * 0.4 ! 2 }.play; )
x.free;

// 3) x8 — extreme harmonic thickening
( x = { WavesetMultiply.ar(b, 8) * 0.3 ! 2 }.play; )
x.free;

// 4) sweep the multiplier 1..6
(
x = { WavesetMultiply.ar(b, LFSaw.kr(0.2).range(1, 6).round) * 0.35 ! 2 }.play;
)
x.free;

// 5) combine with rate: x3 frequency, then play back an octave down
( x = { WavesetMultiply.ar(b, 3, rate: 0.5) * 0.4 ! 2 }.play; )
x.free;
