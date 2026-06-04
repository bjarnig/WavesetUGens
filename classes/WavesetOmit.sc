WavesetOmit : UGen {
	*ar { |bufnum, keep = 1, skip = 1, rate = 1, numCycles = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('audio', bufnum, keep, skip, rate, numCycles, startPos).madd(mul, add)
	}

	*kr { |bufnum, keep = 1, skip = 1, rate = 1, numCycles = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('control', bufnum, keep, skip, rate, numCycles, startPos).madd(mul, add)
	}

	checkInputs { ^this.checkValidInputs }
}
