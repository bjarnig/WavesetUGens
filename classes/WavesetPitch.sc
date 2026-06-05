WavesetPitch : UGen {
	*ar { |bufnum, octvary = 0.3, cyclecnt = 64, seed = 0, rate = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('audio', bufnum, octvary, cyclecnt, seed, rate, startPos).madd(mul, add)
	}

	*kr { |bufnum, octvary = 0.3, cyclecnt = 64, seed = 0, rate = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('control', bufnum, octvary, cyclecnt, seed, rate, startPos).madd(mul, add)
	}

	checkInputs { ^this.checkValidInputs }
}
