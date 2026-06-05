// === WavesetShuffle tests ===
// Reorders wavesets by a domain/image pattern. domain = window of wavesets
// (e.g. "abcd"); image = output order, may reorder/omit/duplicate (duplication
// time-stretches). Use a substantial cyclecnt to make it audible.
s.boot;
b = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav"); // mono

// dry reference
( x = { PlayBuf.ar(1, b, BufRateScale.kr(b), loop: 1) * 0.5 ! 2 }.play; )
x.free;

// 1) reverse groups of 4 (chunks of 20 cycles each)
( x = { WavesetShuffle.ar(b, "abcd", "dcba", 20) * 0.5 ! 2 }.play; )
x.free;

// 2) stutter-stretch: duplicate each element
( x = { WavesetShuffle.ar(b, "abc", "aabbcc", 30) * 0.5 ! 2 }.play; )
x.free;

// 3) omit + reorder: drop some, jumble the rest
( x = { WavesetShuffle.ar(b, "abcd", "dba", 24) * 0.5 ! 2 }.play; )
x.free;

// 4) numeric form: domain size 3, explicit index pattern
( x = { WavesetShuffle.ar(b, 3, [2, 0, 1, 1, 0], 24) * 0.5 ! 2 }.play; )
x.free;

// 5) busy reshuffle of a larger window
( x = { WavesetShuffle.ar(b, "abcdef", "afbecd", 16) * 0.5 ! 2 }.play; )
x.free;
