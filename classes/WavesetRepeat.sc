WavesetRepeat : UGen {
	// bufnum    : a MONO buffer holding the source sound
	// repeats   : how many times each waveset is replayed (>= 1)
	// rate      : playback rate of each waveset (transposition; > 0)
	// numCycles : full wavecycles per waveset (CDP "cyclecnt"; >= 1)
	// startPos  : initial read position in frames (read once, at init)
	*ar { |bufnum, repeats = 4, rate = 1, numCycles = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('audio', bufnum, repeats, rate, numCycles, startPos).madd(mul, add)
	}

	*kr { |bufnum, repeats = 4, rate = 1, numCycles = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('control', bufnum, repeats, rate, numCycles, startPos).madd(mul, add)
	}

	checkInputs { ^this.checkValidInputs }
}
