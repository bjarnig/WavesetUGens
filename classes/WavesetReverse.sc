WavesetReverse : UGen {
	*ar { |bufnum, rate = 1, numCycles = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('audio', bufnum, rate, numCycles, startPos).madd(mul, add)
	}

	*kr { |bufnum, rate = 1, numCycles = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('control', bufnum, rate, numCycles, startPos).madd(mul, add)
	}

	checkInputs { ^this.checkValidInputs }
}
