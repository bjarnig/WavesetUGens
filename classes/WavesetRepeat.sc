WavesetRepeat : UGen {
	// bufnum     : a MONO buffer holding the source sound
	// repeats    : how many times each waveset is replayed (>= 1)
	// rate       : playback rate of each waveset (transposition; > 0)
	// numCycles  : full wavecycles per waveset (CDP "cyclecnt"; >= 1)
	// startPos   : initial read position in frames (read once, at init)
	// oversample : anti-aliasing oversampling 1/2/4/8 (read once; 1 = off)
	*ar { |bufnum, repeats = 4, rate = 1, numCycles = 1, startPos = 0, oversample = 1, mul = 1, add = 0|
		^this.multiNew('audio', bufnum, repeats, rate, numCycles, startPos, oversample).madd(mul, add)
	}

	*kr { |bufnum, repeats = 4, rate = 1, numCycles = 1, startPos = 0, oversample = 1, mul = 1, add = 0|
		^this.multiNew('control', bufnum, repeats, rate, numCycles, startPos, oversample).madd(mul, add)
	}

	checkInputs { ^this.checkValidInputs }
}
