WavesetMultiply : UGen {
	*ar { |bufnum, factor = 2, rate = 1, numCycles = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('audio', bufnum, factor, rate, numCycles, startPos).madd(mul, add)
	}

	*kr { |bufnum, factor = 2, rate = 1, numCycles = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('control', bufnum, factor, rate, numCycles, startPos).madd(mul, add)
	}

	checkInputs { ^this.checkValidInputs }
}
