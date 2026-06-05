// === WavesetFractal tests ===
// Adds a miniature copy of the whole `scale`-cycle group into each constituent
// cycle (self-similar detail), scaled by `amp`. Length preserved.
s.boot;
b = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav"); // mono

// dry reference
( x = { PlayBuf.ar(1, b, BufRateScale.kr(b), loop: 1) * 0.5 ! 2 }.play; )
x.free;

// 1) subtle detail (low amp)
( x = { WavesetFractal.ar(b, 4, 0.25) * 0.4 ! 2 }.play; )
x.free;

// 2) stronger perturbation
( x = { WavesetFractal.ar(b, 4, 0.6) * 0.35 ! 2 }.play; )
x.free;

// 3) larger group as the "model" shape
( x = { WavesetFractal.ar(b, 16, 0.5) * 0.35 ! 2 }.play; )
x.free;

// 4) animate the perturbation amount
(
x = { WavesetFractal.ar(b, 8, SinOsc.kr(0.3).range(0.0, 0.8)) * 0.35 ! 2 }.play;
)
x.free;

// 5) fractal + transpose
( x = { WavesetFractal.ar(b, 6, 0.5, rate: 1.25) * 0.35 ! 2 }.play; )
x.free;
