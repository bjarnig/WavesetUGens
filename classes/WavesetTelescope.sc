WavesetTelescope : UGen {
	*ar { |bufnum, cyclecnt = 4, useMean = 0, rate = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('audio', bufnum, cyclecnt, useMean, rate, startPos).madd(mul, add)
	}

	*kr { |bufnum, cyclecnt = 4, useMean = 0, rate = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('control', bufnum, cyclecnt, useMean, rate, startPos).madd(mul, add)
	}

	checkInputs { ^this.checkValidInputs }
}
