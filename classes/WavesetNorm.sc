WavesetNorm : UGen {
	*ar { |bufnum, amp = 1, rate = 1, numCycles = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('audio', bufnum, amp, rate, numCycles, startPos).madd(mul, add)
	}

	*kr { |bufnum, amp = 1, rate = 1, numCycles = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('control', bufnum, amp, rate, numCycles, startPos).madd(mul, add)
	}

	checkInputs { ^this.checkValidInputs }
}
