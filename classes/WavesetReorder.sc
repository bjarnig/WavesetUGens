WavesetReorder : UGen {
	*ar { |bufnum, groupSize = 4, rate = 1, numCycles = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('audio', bufnum, groupSize, rate, numCycles, startPos).madd(mul, add)
	}

	*kr { |bufnum, groupSize = 4, rate = 1, numCycles = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('control', bufnum, groupSize, rate, numCycles, startPos).madd(mul, add)
	}

	checkInputs { ^this.checkValidInputs }
}
