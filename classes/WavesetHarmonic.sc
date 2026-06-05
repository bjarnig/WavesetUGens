WavesetHarmonic : UGen {
	// harmonics : array of harmonic numbers to add (e.g. [2, 3, 4])
	// amps      : matching amplitudes (extended with 0 if shorter)
	*ar { |bufnum, harmonics = #[2, 3], amps = #[0.5, 0.25], rate = 1, startPos = 0, mul = 1, add = 0|
		var h = harmonics.asArray, a = amps.asArray;
		a = a.extend(h.size, 0);
		^this.multiNew('audio', bufnum, rate, startPos, h.size, *(h ++ a)).madd(mul, add)
	}

	*kr { |bufnum, harmonics = #[2, 3], amps = #[0.5, 0.25], rate = 1, startPos = 0, mul = 1, add = 0|
		var h = harmonics.asArray, a = amps.asArray;
		a = a.extend(h.size, 0);
		^this.multiNew('control', bufnum, rate, startPos, h.size, *(h ++ a)).madd(mul, add)
	}

	checkInputs { ^this.checkValidInputs }
}
