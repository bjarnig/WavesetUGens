WavesetDelete : UGen {
	*ar { |bufnum, mode = 1, cyclecnt = 2, rate = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('audio', bufnum, mode, cyclecnt, rate, startPos).madd(mul, add)
	}

	*kr { |bufnum, mode = 1, cyclecnt = 2, rate = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('control', bufnum, mode, cyclecnt, rate, startPos).madd(mul, add)
	}

	checkInputs { ^this.checkValidInputs }
}
