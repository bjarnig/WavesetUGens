// === WavesetInterpolate tests ===
// Morphs between consecutive wavesets, emitting `multiplier` crossfaded cycles
// per source cycle (a waveset time-stretch that smooths transitions).
s.boot;
b = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav"); // mono

// dry reference
( x = { PlayBuf.ar(1, b, BufRateScale.kr(b), loop: 1) * 0.5 ! 2 }.play; )
x.free;

// 1) x2 — gentle stretch, smoother than WavesetRepeat
( x = { WavesetInterpolate.ar(b, 2) * 0.5 ! 2 }.play; )
x.free;

// 2) x4 — slower, liquid morphing
( x = { WavesetInterpolate.ar(b, 4) * 0.5 ! 2 }.play; )
x.free;

// 3) x12 — extreme smear
( x = { WavesetInterpolate.ar(b, 12) * 0.5 ! 2 }.play; )
x.free;

// 4) longer wavesets (cyclecnt) morph between bigger grains
( x = { WavesetInterpolate.ar(b, 6, cyclecnt: 4) * 0.5 ! 2 }.play; )
x.free;

// 5) morph + transpose up
( x = { WavesetInterpolate.ar(b, 4, rate: 1.5) * 0.45 ! 2 }.play; )
x.free;
