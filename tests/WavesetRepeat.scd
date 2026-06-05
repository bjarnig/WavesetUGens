// === WavesetRepeat tests ===
// Replays each waveset `multiplier` times before advancing (time-stretch + grit).
s.boot;
b = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav"); // mono

// dry reference
( x = { PlayBuf.ar(1, b, BufRateScale.kr(b), loop: 1) * 0.5 ! 2 }.play; )
x.free;

// 1) light: each waveset played twice
( x = { WavesetRepeat.ar(b, multiplier: 2) * 0.5 ! 2 }.play; )
x.free;

// 2) heavy stretch: 8 repeats -> long, buzzy
( x = { WavesetRepeat.ar(b, multiplier: 8) * 0.5 ! 2 }.play; )
x.free;

// 3) longer wavesets (cyclecnt) keep more of the source's identity
( x = { WavesetRepeat.ar(b, multiplier: 6, cyclecnt: 4) * 0.5 ! 2 }.play; )
x.free;

// 4) transpose each waveset up a fifth while repeating
( x = { WavesetRepeat.ar(b, multiplier: 4, rate: 1.5) * 0.4 ! 2 }.play; )
x.free;

// 5) modulate the repeat count for shifting texture
(
x = {
	WavesetRepeat.ar(b, multiplier: LFNoise0.kr(4).range(1, 12).round) * 0.4 ! 2
}.play;
)
x.free;
