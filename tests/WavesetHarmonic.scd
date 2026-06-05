// === WavesetHarmonic tests ===
// Adds harmonics over each wavecycle: for each harmonic number h with amplitude
// a, a copy of the cycle read h x faster (wrapped) is summed in. Harmonics near
// len/4 are skipped (foldover guard).
s.boot;
b = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav"); // mono

// dry reference
( x = { PlayBuf.ar(1, b, BufRateScale.kr(b), loop: 1) * 0.5 ! 2 }.play; )
x.free;

// 1) add 2nd + 3rd harmonics
( x = { WavesetHarmonic.ar(b, [2, 3], [0.5, 0.25]) * 0.35 ! 2 }.play; )
x.free;

// 2) odd harmonics only — squarer, hollow
( x = { WavesetHarmonic.ar(b, [3, 5, 7], [0.4, 0.3, 0.2]) * 0.3 ! 2 }.play; )
x.free;

// 3) a fuller stack
( x = { WavesetHarmonic.ar(b, [2, 3, 4, 5, 6], [0.5, 0.4, 0.3, 0.2, 0.15]) * 0.25 ! 2 }.play; )
x.free;

// 4) just the octave, loud
( x = { WavesetHarmonic.ar(b, [2], [0.9]) * 0.35 ! 2 }.play; )
x.free;

// 5) harmonics + transpose down to feature the added partials
( x = { WavesetHarmonic.ar(b, [2, 3, 5], [0.5, 0.3, 0.2], rate: 0.5) * 0.3 ! 2 }.play; )
x.free;
