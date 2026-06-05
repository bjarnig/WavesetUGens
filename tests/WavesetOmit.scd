// === WavesetOmit tests ===
// Silences `omit` of every `outOf` wavesets (timing preserved).
s.boot;
b = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav"); // mono

// dry reference
( x = { PlayBuf.ar(1, b, BufRateScale.kr(b), loop: 1) * 0.5 ! 2 }.play; )
x.free;

// 1) silence 1 of every 2 — gappy
( x = { WavesetOmit.ar(b, omit: 1, outOf: 2) * 0.5 ! 2 }.play; )
x.free;

// 2) silence 3 of every 4 — sparse, stuttery
( x = { WavesetOmit.ar(b, omit: 3, outOf: 4) * 0.5 ! 2 }.play; )
x.free;

// 3) bigger wavesets so the gaps are chunkier
( x = { WavesetOmit.ar(b, omit: 2, outOf: 3, cyclecnt: 8) * 0.5 ! 2 }.play; )
x.free;

// 4) rhythmic gating: modulate how many are silenced
(
x = { WavesetOmit.ar(b, LFNoise0.kr(3).range(0, 5).round, outOf: 6) * 0.5 ! 2 }.play;
)
x.free;

// 5) omit + transpose
( x = { WavesetOmit.ar(b, 1, 3, rate: 1.5) * 0.5 ! 2 }.play; )
x.free;
