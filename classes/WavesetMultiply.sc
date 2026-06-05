WavesetMultiply : UGen {
	*ar { |bufnum, multiplier = 2, rate = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('audio', bufnum, multiplier, rate, startPos).madd(mul, add)
	}

	*kr { |bufnum, multiplier = 2, rate = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('control', bufnum, multiplier, rate, startPos).madd(mul, add)
	}

	checkInputs { ^this.checkValidInputs }
}
