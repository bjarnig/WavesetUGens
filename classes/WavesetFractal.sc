WavesetFractal : UGen {
	*ar { |bufnum, scale = 4, amp = 0.5, rate = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('audio', bufnum, scale, amp, rate, startPos).madd(mul, add)
	}

	*kr { |bufnum, scale = 4, amp = 0.5, rate = 1, startPos = 0, mul = 1, add = 0|
		^this.multiNew('control', bufnum, scale, amp, rate, startPos).madd(mul, add)
	}

	checkInputs { ^this.checkValidInputs }
}
