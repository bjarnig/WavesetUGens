WavesetReplace : UGen {
	*ar { |bufnum, cyclecnt = 2, rate = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('audio', bufnum, cyclecnt, rate, startPos).madd(mul, add)
	}

	*kr { |bufnum, cyclecnt = 2, rate = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('control', bufnum, cyclecnt, rate, startPos).madd(mul, add)
	}

	checkInputs { ^this.checkValidInputs }
}
