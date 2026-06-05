WavesetDivide : UGen {
	*ar { |bufnum, divider = 2, rate = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('audio', bufnum, divider, rate, startPos).madd(mul, add)
	}

	*kr { |bufnum, divider = 2, rate = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('control', bufnum, divider, rate, startPos).madd(mul, add)
	}

	checkInputs { ^this.checkValidInputs }
}
