// === WavesetDelete tests ===
// Drops wavecycles, contracting time. mode 1 = keep one then delete cyclecnt;
// 2 = keep loudest of cyclecnt; 3 = drop quietest of cyclecnt.
s.boot;
b = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav"); // mono

// dry reference
( x = { PlayBuf.ar(1, b, BufRateScale.kr(b), loop: 1) * 0.5 ! 2 }.play; )
x.free;

// 1) strict order: keep 1, delete 1 — twice as fast
( x = { WavesetDelete.ar(b, mode: 1, cyclecnt: 1) * 0.5 ! 2 }.play; )
x.free;

// 2) strict order: keep 1, delete 3 — strong time-compression
( x = { WavesetDelete.ar(b, mode: 1, cyclecnt: 3) * 0.5 ! 2 }.play; )
x.free;

// 3) keep the loudest of every 4 cycles
( x = { WavesetDelete.ar(b, mode: 2, cyclecnt: 4) * 0.5 ! 2 }.play; )
x.free;

// 4) drop the quietest of every 3 cycles (subtler)
( x = { WavesetDelete.ar(b, mode: 3, cyclecnt: 3) * 0.5 ! 2 }.play; )
x.free;

// 5) modulate the delete count in strict mode
(
x = { WavesetDelete.ar(b, mode: 1, cyclecnt: LFNoise0.kr(2).range(1, 6).round) * 0.5 ! 2 }.play;
)
x.free;
