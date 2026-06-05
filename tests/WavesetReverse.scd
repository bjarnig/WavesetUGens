// === WavesetReverse tests ===
// Plays each group of `cyclecnt` wavecycles backwards (sample reversal).
s.boot;
b = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav"); // mono

// dry reference
( x = { PlayBuf.ar(1, b, BufRateScale.kr(b), loop: 1) * 0.5 ! 2 }.play; )
x.free;

// 1) small groups — subtle roughening
( x = { WavesetReverse.ar(b, 2) * 0.5 ! 2 }.play; )
x.free;

// 2) bigger groups — clear backwards chunks
( x = { WavesetReverse.ar(b, 16) * 0.5 ! 2 }.play; )
x.free;

// 3) large groups — recognisable reversed fragments
( x = { WavesetReverse.ar(b, 64) * 0.5 ! 2 }.play; )
x.free;

// 4) reverse + transpose down an octave
( x = { WavesetReverse.ar(b, 16, rate: 0.5) * 0.5 ! 2 }.play; )
x.free;

// 5) modulate the group size
(
x = { WavesetReverse.ar(b, LFNoise0.kr(2).range(2, 48).round) * 0.5 ! 2 }.play;
)
x.free;
