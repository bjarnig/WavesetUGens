WavesetOmit : UGen {
	*ar { |bufnum, omit = 1, outOf = 2, rate = 1, cyclecnt = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('audio', bufnum, omit, outOf, rate, cyclecnt, startPos).madd(mul, add)
	}

	*kr { |bufnum, omit = 1, outOf = 2, rate = 1, cyclecnt = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('control', bufnum, omit, outOf, rate, cyclecnt, startPos).madd(mul, add)
	}

	checkInputs { ^this.checkValidInputs }
}
