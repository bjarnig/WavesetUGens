WavesetInterpolate : UGen {
	*ar { |bufnum, stretch = 2, rate = 1, numCycles = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('audio', bufnum, stretch, rate, numCycles, startPos).madd(mul, add)
	}

	*kr { |bufnum, stretch = 2, rate = 1, numCycles = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('control', bufnum, stretch, rate, numCycles, startPos).madd(mul, add)
	}

	checkInputs { ^this.checkValidInputs }
}
