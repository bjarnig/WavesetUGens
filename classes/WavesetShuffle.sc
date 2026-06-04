WavesetShuffle : UGen {
	*ar { |bufnum, groupSize = 4, seed = 0, rate = 1, numCycles = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('audio', bufnum, groupSize, seed, rate, numCycles, startPos).madd(mul, add)
	}

	*kr { |bufnum, groupSize = 4, seed = 0, rate = 1, numCycles = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('control', bufnum, groupSize, seed, rate, numCycles, startPos).madd(mul, add)
	}

	checkInputs { ^this.checkValidInputs }
}
