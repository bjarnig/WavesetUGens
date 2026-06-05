// === WavesetDivide tests ===
// Divides wavecycle frequency by `divider` (pitch down, duration preserved).
s.boot;
b = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav"); // mono

// dry reference
( x = { PlayBuf.ar(1, b, BufRateScale.kr(b), loop: 1) * 0.5 ! 2 }.play; )
x.free;

// 1) /2 — down an octave in frequency, same duration
( x = { WavesetDivide.ar(b, 2) * 0.5 ! 2 }.play; )
x.free;

// 2) /4 — lower, rougher
( x = { WavesetDivide.ar(b, 4) * 0.5 ! 2 }.play; )
x.free;

// 3) /8 — extreme, sub-tone texture
( x = { WavesetDivide.ar(b, 8) * 0.5 ! 2 }.play; )
x.free;

// 4) sweep the divider 1..6
(
x = { WavesetDivide.ar(b, LFSaw.kr(0.2).range(1, 6).round) * 0.5 ! 2 }.play;
)
x.free;

// 5) divide then transpose back up — keeps the rough texture, restores register
( x = { WavesetDivide.ar(b, 4, rate: 2) * 0.5 ! 2 }.play; )
x.free;
