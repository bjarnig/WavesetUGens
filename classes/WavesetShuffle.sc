WavesetShuffle : UGen {
	// domain : a letter set ("abcd") or an integer window size
	// image  : a permutation over the domain ("aaccbd" or [0,0,2,2,1,3]); may
	//          reorder, omit, or duplicate domain elements. nil = identity.
	*ar { |bufnum, domain = 4, image, cyclecnt = 1, rate = 1, startPos = 0, mul = 1, add = 0|
		var d, idx;
		# d, idx = this.prParse(domain, image);
		^this.multiNew('audio', bufnum, cyclecnt, rate, startPos, d, idx.size, *idx).madd(mul, add)
	}

	*kr { |bufnum, domain = 4, image, cyclecnt = 1, rate = 1, startPos = 0, mul = 1, add = 0|
		var d, idx;
		# d, idx = this.prParse(domain, image);
		^this.multiNew('control', bufnum, cyclecnt, rate, startPos, d, idx.size, *idx).madd(mul, add)
	}

	*prParse { |domain, image|
		var dsize, idx, dstr;
		(domain.isString or: { domain.isKindOf(Symbol) }).if({
			dstr = domain.asString;
			dsize = dstr.size;
			idx = image.isNil.if(
				{ (0 .. dsize - 1).asArray },
				{ image.asString.collectAs({ |ch| dstr.indexOf(ch) ? 0 }, Array) }
			);
		}, {
			dsize = domain.asInteger;
			idx = image.isNil.if(
				{ (0 .. dsize - 1).asArray },
				{ image.asArray.collect { |x| x.asInteger } }
			);
		});
		if(idx.size < 1) { idx = [0] };
		^[dsize, idx]
	}

	checkInputs { ^this.checkValidInputs }
}
