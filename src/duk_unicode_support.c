/*
 *  Various Unicode help functions for character classification predicates,
 *  case conversion, decoding, etc.
 */

#include "duk_internal.h"

/*
 *  XUTF-8 and CESU-8 encoding/decoding
 */

int duk_unicode_get_xutf8_length(duk_uint32_t x) {
	if (x < 0x80) {
		/* 7 bits */
		return 1;
	} else if (x < 0x800) {
		/* 11 bits */
		return 2;
	} else if (x < 0x10000) {
		/* 16 bits */
		return 3;
	} else if (x < 0x200000) {
		/* 21 bits */
		return 4;
	} else if (x < 0x4000000) {
		/* 26 bits */
		return 5;
	} else if (x < (duk_uint32_t) 0x80000000L) {
		/* 31 bits */
		return 6;
	} else {
		/* 36 bits */
		return 7;
	}
}

duk_uint8_t duk_unicode_xutf8_markers[7] = {
	0x00, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe
};

/* Encode to extended UTF-8; 'out' must have space for at least
 * DUK_UNICODE_MAX_XUTF8_LENGTH bytes.  Allows encoding of any
 * 32-bit (unsigned) codepoint.
 */
size_t duk_unicode_encode_xutf8(duk_uint32_t x, duk_uint8_t *out) {
	size_t len;
	duk_uint8_t marker;
	size_t i;

	len = duk_unicode_get_xutf8_length(x);
	DUK_ASSERT(len > 0);

	marker = duk_unicode_xutf8_markers[len - 1];  /* 64-bit OK because always >= 0 */

	i = len;
	DUK_ASSERT(i > 0);
	do {
		i--;
		if (i > 0) {
			out[i] = 0x80 + (x & 0x3f);
			x >>= 6;
		} else {
			/* Note: masking of 'x' is not necessary because of
			 * range check and shifting -> no bits overlapping
			 * the marker should be set.
			 */
			out[0] = marker + x;
		}
	} while(i > 0);

	return len;
}

/* Encode to CESU-8; 'out' must have space for at least
 * DUK_UNICODE_MAX_CESU8_LENGTH bytes; codepoints above U+10FFFF
 * will encode to garbage but won't overwrite the output buffer.
 */
size_t duk_unicode_encode_cesu8(duk_uint32_t x, duk_uint8_t *out) {
	size_t len;

	if (x < 0x80) {
		out[0] = x;
		len = 1;
	} else if (x < 0x800) {
		out[0] = 0xc0 + ((x >> 6) & 0x1f);
		out[1] = 0x80 + (x & 0x3f);
		len = 2;
	} else if (x < 0x10000) {
		/* surrogate pairs get encoded here */
		out[0] = 0xe0 + ((x >> 12) & 0x0f);
		out[1] = 0x80 + ((x >> 6) & 0x3f);
		out[2] = 0x80 + (x & 0x3f);
		len = 3;
	} else {
		/*
		 *  Unicode codepoints above U+FFFF are encoded as surrogate
		 *  pairs here.  This ensures that all CESU-8 codepoints are
		 *  16-bit values as expected in Ecmascript.  The surrogate
		 *  pairs always get a 3-byte encoding (each) in CESU-8.
		 *  See: http://en.wikipedia.org/wiki/Surrogate_pair
		 *
		 *  20-bit codepoint, 10 bits (A and B) per surrogate pair:
		 * 
		 *    x = 0b00000000 0000AAAA AAAAAABB BBBBBBBB
		 *  sp1 = 0b110110AA AAAAAAAA  (0xd800 + ((x >> 10) & 0x3ff))
		 *  sp2 = 0b110111BB BBBBBBBB  (0xdc00 + (x & 0x3ff))
		 *
		 *  Encoded into CESU-8:
		 *
		 *  sp1 -> 0b11101101  (0xe0 + ((sp1 >> 12) & 0x0f))
		 *      -> 0b1010AAAA  (0x80 + ((sp1 >> 6) & 0x3f))
		 *      -> 0b10AAAAAA  (0x80 + (sp1 & 0x3f))
		 *  sp2 -> 0b11101101  (0xe0 + ((sp2 >> 12) & 0x0f))
		 *      -> 0b1011BBBB  (0x80 + ((sp2 >> 6) & 0x3f))
		 *      -> 0b10BBBBBB  (0x80 + (sp2 & 0x3f))
		 *
		 *  Note that 0x10000 must be subtracted first.  The code below
		 *  avoids the sp1, sp2 temporaries which saves around 20 bytes
		 *  of code.
		 */

		x -= 0x10000;

		out[0] = 0xed;
		out[1] = 0xa0 + ((x >> 16) & 0x0f);
		out[2] = 0x80 + ((x >> 10) & 0x3f);
		out[3] = 0xed;
		out[4] = 0xb0 + ((x >> 6) & 0x0f);
		out[5] = 0x80 + (x & 0x3f);
		len = 6;
	}

	return len;
}

/* Decode helper.  Return zero on error. */
int duk_unicode_xutf8_get_u32(duk_hthread *thr, duk_uint8_t **ptr, duk_uint8_t *ptr_start, duk_uint8_t *ptr_end, duk_uint32_t *out_cp) {
	duk_uint8_t *p;
	duk_uint32_t res;
	int ch;
	int n;

	p = *ptr;
	if (p < ptr_start || p >= ptr_end) {
		goto fail;
	}

	/*
	 *  UTF-8 decoder which accepts longer than standard byte sequences.
	 *  This allows full 32-bit code points to be used.
	 */

	ch = *p++;
	if (ch < 0x80) {
		/* 0xxx xxxx   [7 bits] */
		res = ch & 0x7f;
		n = 0;
	} else if (ch < 0xc0) {
		/* 10xx xxxx -> invalid */
		goto fail;
	} else if (ch < 0xe0) {
		/* 110x xxxx   10xx xxxx   [11 bits] */
		res = ch & 0x1f;
		n = 1;
	} else if (ch < 0xf0) {
		/* 1110 xxxx   10xx xxxx   10xx xxxx   [16 bits] */
		res = ch & 0x0f;
		n = 2;
	} else if (ch < 0xf8) {
		/* 1111 0xxx   10xx xxxx   10xx xxxx   10xx xxxx   [21 bits] */
		res = ch & 0x07;
		n = 3;
	} else if (ch < 0xfc) {
		/* 1111 10xx   10xx xxxx   10xx xxxx   10xx xxxx   10xx xxxx   [26 bits] */
		res = ch & 0x03;
		n = 4;
	} else if (ch < 0xfe) {
		/* 1111 110x   10xx xxxx   10xx xxxx   10xx xxxx   10xx xxxx   10xx xxxx   [31 bits] */
		res = ch & 0x01;
		n = 5;
	} else if (ch < 0xff) {
		/* 1111 1110   10xx xxxx   10xx xxxx   10xx xxxx   10xx xxxx   10xx xxxx   10xx xxxx   [36 bits] */
		res = 0;
		n = 6;
	} else {
		/* 8-byte format could be:
		 * 1111 1111   10xx xxxx   10xx xxxx   10xx xxxx   10xx xxxx   10xx xxxx   10xx xxxx   10xx xxxx   [41 bits]
		 *
		 * However, this format would not have a zero bit following the
		 * leading one bits and would not allow 0xFF to be used as an
		 * "invalid xutf-8" marker for internal keys.  Further, 8-byte
		 * encodings (up to 41 bit code points) are not currently needed.
		 */
		goto fail;
	}

	DUK_ASSERT(p >= ptr_start);  /* verified at beginning */
	if (p + n > ptr_end) {
		/* check pointer at end */
		goto fail;
	}

	while (n > 0) {
		DUK_ASSERT(p >= ptr_start && p < ptr_end);
		res = res << 6;
		res += (*p++) & 0x3f;
		n--;
	}

	*ptr = p;
	*out_cp = res;
	return 1;

 fail:
	return 0;
}

/* used by e.g. duk_regexp_executor.c, string built-ins */
duk_uint32_t duk_unicode_xutf8_get_u32_checked(duk_hthread *thr, duk_uint8_t **ptr, duk_uint8_t *ptr_start, duk_uint8_t *ptr_end) {
	duk_uint32_t cp;

	if (duk_unicode_xutf8_get_u32(thr, ptr, ptr_start, ptr_end, &cp)) {
		return cp;
	}
	DUK_ERROR(thr, DUK_ERR_INTERNAL_ERROR, "utf-8 decode failed");
	return 0;  /* never here */
}

/* (extended) utf-8 length without codepoint encoding validation, used
 * for string interning (should probably be inlined).
 */
duk_uint32_t duk_unicode_unvalidated_utf8_length(duk_uint8_t *data, duk_uint32_t blen) {
        duk_uint8_t *p = data;
        duk_uint8_t *p_end = data + blen;
        duk_uint32_t clen = 0;

        while (p < p_end) {
                duk_uint8_t x = *p++;
                if (x < 0x80) {
                        clen++;
                } else if (x >= 0xc0 ) {
                        /* 10xxxxxx = continuation chars (0x80...0xbf), above that
                         * initial bytes.
                         */
                        clen++;
                }
        }

        return clen;
}

/*
 *  Unicode range matcher
 *
 *  Matches a codepoint against a packed bitstream of character ranges.
 *  Used for slow path Unicode matching.
 */

/* Must match src/extract_chars.py, generate_match_table3(). */
static int uni_decode_value(duk_bitdecoder_ctx *bd_ctx) {
	int t;

	t = duk_bd_decode(bd_ctx, 4);
	if (t <= 0x0e) {
		return t;
	}
	t = duk_bd_decode(bd_ctx, 8);
	if (t <= 0xfd) {
		return t + 0x0f;
	}
	if (t == 0xfe) {
		t = duk_bd_decode(bd_ctx, 12);
		return t + 0x0f + 0xfe;
	} else {
		t = duk_bd_decode(bd_ctx, 24);
		return t + 0x0f + 0xfe + 0x1000;
	}
}

static int uni_range_match(duk_uint8_t *unitab, int unilen, int x) {
	duk_bitdecoder_ctx bd_ctx;

	DUK_MEMSET(&bd_ctx, 0, sizeof(bd_ctx));
	bd_ctx.data = (duk_uint8_t *) unitab;
	bd_ctx.length = (duk_size_t) unilen;

	int prev_re = 0;
	for (;;) {
		int r1, r2;
		r1 = uni_decode_value(&bd_ctx);
		if (r1 == 0) {
			break;
		}
		r2 = uni_decode_value(&bd_ctx);

		r1 = prev_re + r1;
		r2 = r1 + r2;
		prev_re = r2;

		/* [r1,r2] is the range */

		DUK_DDDPRINT("uni_range_match: range=[0x%06x,0x%06x]", r1, r2);
		if (x >= r1 && x <= r2) {
			return 1;
		}
	}

	return 0;
}

/*
 *  "WhiteSpace" production check.
 */

int duk_unicode_is_whitespace(int x) {
	/*
	 *  E5 Section 7.2 specifies six characters specifically as
	 *  white space:
	 *
	 *    0009;<control>;Cc;0;S;;;;;N;CHARACTER TABULATION;;;;
	 *    000B;<control>;Cc;0;S;;;;;N;LINE TABULATION;;;;
	 *    000C;<control>;Cc;0;WS;;;;;N;FORM FEED (FF);;;;
	 *    0020;SPACE;Zs;0;WS;;;;;N;;;;;
	 *    00A0;NO-BREAK SPACE;Zs;0;CS;<noBreak> 0020;;;;N;NON-BREAKING SPACE;;;;
	 *    FEFF;ZERO WIDTH NO-BREAK SPACE;Cf;0;BN;;;;;N;BYTE ORDER MARK;;;;
	 *
	 *  It also specifies any Unicode category 'Z' characters as white
	 *  space.  These can be extracted with the "src/extract_chars.py" script,
	 *  see src/SConscript t_uni_ws target.
	 *
	 *  Current result (built as WhiteSpace-Z.txt).
	 *
	 *    RAW OUTPUT:
	 *    ===========
	 *    0020;SPACE;Zs;0;WS;;;;;N;;;;;
	 *    00A0;NO-BREAK SPACE;Zs;0;CS;<noBreak> 0020;;;;N;NON-BREAKING SPACE;;;;
	 *    1680;OGHAM SPACE MARK;Zs;0;WS;;;;;N;;;;;
	 *    180E;MONGOLIAN VOWEL SEPARATOR;Zs;0;WS;;;;;N;;;;;
	 *    2000;EN QUAD;Zs;0;WS;2002;;;;N;;;;;
	 *    2001;EM QUAD;Zs;0;WS;2003;;;;N;;;;;
	 *    2002;EN SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
	 *    2003;EM SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
	 *    2004;THREE-PER-EM SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
	 *    2005;FOUR-PER-EM SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
	 *    2006;SIX-PER-EM SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
	 *    2007;FIGURE SPACE;Zs;0;WS;<noBreak> 0020;;;;N;;;;;
	 *    2008;PUNCTUATION SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
	 *    2009;THIN SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
	 *    200A;HAIR SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
	 *    2028;LINE SEPARATOR;Zl;0;WS;;;;;N;;;;;
	 *    2029;PARAGRAPH SEPARATOR;Zp;0;B;;;;;N;;;;;
	 *    202F;NARROW NO-BREAK SPACE;Zs;0;CS;<noBreak> 0020;;;;N;;;;;
	 *    205F;MEDIUM MATHEMATICAL SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
	 *    3000;IDEOGRAPHIC SPACE;Zs;0;WS;<wide> 0020;;;;N;;;;;
	 *    
	 *    RANGES:
	 *    =======
	 *    0x0020
	 *    0x00a0
	 *    0x1680
	 *    0x180e
	 *    0x2000 ... 0x200a
	 *    0x2028 ... 0x2029
	 *    0x202f
	 *    0x205f
	 *    0x3000
	 *
	 *  A manual decoder (below) is probably most compact for this.
	 */

	unsigned char lo;
	int hi;

	lo = (unsigned char) (x & 0xff);
	hi = (int) (x >> 8);  /* does not fit into an uchar */

	if (hi == 0x0000) {
		if (lo == 0x09 || lo == 0x0b || lo == 0x0c ||
	            lo == 0x20 || lo == 0xa0) {
			return 1;
		}
	} else if (hi == 0x0020) {
		if (lo <= 0x0a || lo == 0x28 || lo == 0x29 ||
		    lo == 0x2f || lo == 0x5f) {
			return 1;
		}
	} else if (x == 0x1680 || x == 0x180e || x == 0x3000 ||
	           x == 0xfeff) {
		return 1;
	}

	return 0;
}

/*
 *  "LineTerminator" production check.
 */

int duk_unicode_is_line_terminator(int x) {
	/*
	 *  E5 Section 7.3
	 *
	 *  A LineTerminatorSequence essentially merges <CR> <LF> sequences
	 *  into a single line terminator.  This must be handled by the caller.
	 */

	if (x == 0x000a || x == 0x000d || x == 0x2028 ||
	    x == 0x2029) {
		return 1;
	}

	return 0;
}

/*
 *  "IdentifierStart" production check.
 */

int duk_unicode_is_identifier_start(int x) {
	/*
	 *  E5 Section 7.6:
	 *
	 *    IdentifierStart:
	 *      UnicodeLetter
	 *      $
	 *      _
	 *      \ UnicodeEscapeSequence
	 *
	 *  IdentifierStart production has one multi-character production:
	 *
	 *    \ UnicodeEscapeSequence
	 *
	 *  The '\' character is -not- matched by this function.  Rather, the caller
	 *  should decode the escape and then call this function to check whether the
	 *  decoded character is acceptable (see discussion in E5 Section 7.6).
	 *
	 *  The "UnicodeLetter" alternative of the production allows letters
	 *  from various Unicode categories.  These can be extracted with the
	 *  "src/extract_chars.py" script, see src/SConscript t_uni_idstart* targets.
	 *
	 *  Because the result has hundreds of Unicode codepoint ranges, matching
	 *  for any values >= 0x80 are done using a very slow range-by-range scan
	 *  and a packed range format.
	 *
	 *  The ASCII portion (codepoints 0x00 ... 0x7f) is fast-pathed below because
	 *  it matters the most.  The ASCII related ranges of IdentifierStart are:
	 *
	 *    0x0041 ... 0x005a		['A' ... 'Z']
	 *    0x0061 ... 0x007a		['a' ... 'z']
	 *    0x0024			['$']
	 *    0x005f			['_']
	 */

	/* ASCII fast path -- quick accept and reject */
	if (x <= 0x7f) {
		if ((x >= 'a' && x <= 'z') ||
		    (x >= 'A' && x <= 'Z') ||
		    x == '_' || x == '$') {
			return 1;
		}
		return 0;
	}

	/* Non-ASCII slow path (range-by-range linear comparison), very slow */

#ifdef DUK_USE_SOURCE_NONBMP
	if (uni_range_match(duk_unicode_identifier_start_noascii,
	                    sizeof(duk_unicode_identifier_start_noascii),
	                    x)) {
		return 1;
	}
	return 0;
#else
	if (x < 0x10000) {
		if (uni_range_match(duk_unicode_identifier_start_noascii_bmponly,
	                    sizeof(duk_unicode_identifier_start_noascii_bmponly),
	                    x)) {
			return 1;
		}
		return 0;
	} else {
		/* without explicit non-BMP support, assume non-BMP characters
		 * are always accepted as identifier characters.
		 */
		return 1;
	}
#endif
}

/*
 *  "IdentifierPart" production check.
 */

int duk_unicode_is_identifier_part(int x) {
	/*
	 *  E5 Section 7.6:
	 *
	 *    IdentifierPart:
	 *      IdentifierStart
	 *      UnicodeCombiningMark
	 *      UnicodeDigit
	 *      UnicodeConnectorPunctuation
	 *      <ZWNJ>	[U+200C]
	 *      <ZWJ>	[U+200D]
	 *
	 *  IdentifierPart production has one multi-character production
	 *  as part of its IdentifierStart alternative.  The '\' character
	 *  of an escape sequence is not matched here, see discussion in
	 *  duk_unicode_is_identifier_start().
	 *
	 *  To match non-ASCII characters (codepoints >= 0x80), a very slow
	 *  linear range-by-range scan is used.  The codepoint is first compared
	 *  to the IdentifierStart ranges, and if it doesn't match, then to a
	 *  set consisting of code points in IdentifierPart but not in
	 *  IdentifierStart.  This is done to keep the unicode range data small,
	 *  at the expense of speed.
	 *
	 *  The ASCII fast path consists of:
	 *
	 *    0x0030 ... 0x0039		['0' ... '9', UnicodeDigit]
	 *    0x0041 ... 0x005a		['A' ... 'Z', IdentifierStart]
	 *    0x0061 ... 0x007a		['a' ... 'z', IdentifierStart]
	 *    0x0024			['$', IdentifierStart]
	 *    0x005f			['_', IdentifierStart and
	 *                               UnicodeConnectorPunctuation]
	 *
	 *  UnicodeCombiningMark has no code points <= 0x7f.
	 *
	 *  The matching code reuses the "identifier start" tables, and then
	 *  consults a separate range set for characters in "identifier part"
	 *  but not in "identifier start".  These can be extracted with the
	 *  "src/extract_chars.py" script, see src/SConscript
	 *  t_uni_idpart_minus_idstart* targets.
	 *
	 *  UnicodeCombiningMark -> categories Mn, Mc
	 *  UnicodeDigit -> categories Nd
	 *  UnicodeConnectorPunctuation -> categories Pc
	 */

	/* ASCII fast path -- quick accept and reject */
	if (x <= 0x7f) {
		if ((x >= 'a' && x <= 'z') ||
		    (x >= 'A' && x <= 'Z') ||
		    (x >= '0' && x <= '9') ||
		    x == '_' || x == '$') {
			return 1;
		}
		return 0;
	}

	/* Non-ASCII slow path (range-by-range linear comparison), very slow */

#ifdef DUK_USE_SOURCE_NONBMP
	if (uni_range_match(duk_unicode_identifier_start_noascii,
	                    sizeof(duk_unicode_identifier_start_noascii),
	                    x) ||
	    uni_range_match(duk_unicode_identifier_part_minus_identifier_start_noascii,
	                    sizeof(duk_unicode_identifier_part_minus_identifier_start_noascii),
	                    x)) {
		return 1;
	}
	return 0;
#else
	if (x < 0x10000) {
		if (uni_range_match(duk_unicode_identifier_start_noascii_bmponly,
		                    sizeof(duk_unicode_identifier_start_noascii_bmponly),
		                    x) ||
		    uni_range_match(duk_unicode_identifier_part_minus_identifier_start_noascii_bmponly,
		                    sizeof(duk_unicode_identifier_part_minus_identifier_start_noascii_bmponly),
		                    x)) {
			return 1;
		}
		return 0;
	} else {
		/* without explicit non-BMP support, assume non-BMP characters
		 * are always accepted as identifier characters.
		 */
		return 1;
	}
#endif
}

/*
 *  Complex case conversion helper which decodes a bit-packed conversion
 *  control stream generated by unicode/extract_caseconv.py.  The conversion
 *  is very slow because it runs through the conversion data in a linear
 *  fashion to save space (which is why ASCII characters have a special
 *  fast path before arriving here).
 * 
 *  The particular bit counts etc have been determined experimentally to
 *  be small but still sufficient, and must match the Python script
 *  (src/extract_caseconv.py).
 *
 *  The return value is the case converted codepoint or -1 if the conversion
 *  results in multiple characters (this is useful for regexp Canonicalization
 *  operation).  If 'buf' is not NULL, the result codepoint(s) are also
 *  appended to the hbuffer.
 *
 *  Context and locale specific rules must be checked before consulting
 *  this function.
 */

static int slow_case_conversion(duk_hthread *thr,
                                duk_hbuffer_dynamic *buf,
                                int x,
                                duk_bitdecoder_ctx *bd_ctx) {
	int skip = 0;
	int n, t;
	int start_i, start_o, count;

	DUK_DDDPRINT("slow case conversion for codepoint: %d", x);

	/* range conversion with a "skip" */
	DUK_DDDPRINT("checking ranges");
	for (;;) {
		skip++;
		n = duk_bd_decode(bd_ctx, 6);
		if (n == 0x3f) {
			/* end marker */
			break;
		}
		DUK_DDDPRINT("skip=%d, n=%d", skip, n);

		while (n--) {
			start_i = duk_bd_decode(bd_ctx, 16);
			start_o = duk_bd_decode(bd_ctx, 16);
			count = duk_bd_decode(bd_ctx, 7);
			DUK_DDDPRINT("range: start_i=%d, start_o=%d, count=%d, skip=%d",
			             start_i, start_o, count, skip);

			t = x - start_i;
			if (t >= 0 && t < count * skip && (t % skip) == 0) {
				DUK_DDDPRINT("range matches input codepoint");
				x = start_o + t;
				goto single;
			}
		}
	}

	/* 1:1 conversion */
	n = duk_bd_decode(bd_ctx, 6);
	DUK_DDDPRINT("checking 1:1 conversions (count %d)", n);
	while (n--) {
		start_i = duk_bd_decode(bd_ctx, 16);
		start_o = duk_bd_decode(bd_ctx, 16);
		DUK_DDDPRINT("1:1 conversion %d -> %d", start_i, start_o);
		if (x == start_i) {
			DUK_DDDPRINT("1:1 matches input codepoint");
			x = start_o;
			goto single;
		}
	}

	/* complex, multicharacter conversion */
	n = duk_bd_decode(bd_ctx, 7);
	DUK_DDDPRINT("checking 1:n conversions (count %d)", n);
	while (n--) {
		start_i = duk_bd_decode(bd_ctx, 16);
		t = duk_bd_decode(bd_ctx, 2);
		DUK_DDDPRINT("1:n conversion %d -> %d chars", start_i, t);
		if (x == start_i) {
			DUK_DDDPRINT("1:n matches input codepoint");
			if (buf) {
				while (t--) {
					int tmp = duk_bd_decode(bd_ctx, 16);
					DUK_ASSERT(buf != NULL);
					duk_hbuffer_append_xutf8(thr, buf, tmp);
				}
			}
			return -1;
		} else {
			while (t--) {
				(void) duk_bd_decode(bd_ctx, 16);
			}
		}
	}

	/* default: no change */
	DUK_DDDPRINT("no rule matches, output is same as input");
	/* fall through */

 single:
	if (buf) {
		duk_hbuffer_append_xutf8(thr, buf, x);
	}
	return x;
}

/*
 *  Case conversion helper, with context/local sensitivity.
 *  For proper case conversion, one needs to know the character
 *  and the preceding and following characters, as well as
 *  locale/language.
 */

static int case_transform_helper(duk_hthread *thr,
                                 duk_hbuffer_dynamic *buf,
                                 int x,
                                 int prev,
                                 int next,
                                 int uppercase,
                                 int language) {
	duk_bitdecoder_ctx bd_ctx;

	/* fast path for ASCII */
	if (x < 0x80) {
		/* FIXME: context sensitive rules exist for ASCII range too.
		 * Need to add them here.
		 */

		if (uppercase) {
			if (x >= 'a' && x <= 'z') {
				x = x - 'a' + 'A';
			}
		} else {
			if (x >= 'A' && x <= 'Z') {
				x = x - 'A' + 'a';
			}
		}
		goto singlechar;
	}

	/* context and locale specific rules which cannot currently be represented
	 * in the caseconv bitstream: hardcoded rules in C
	 */
	if (uppercase) {
		/* FIXME: turkish / azeri */
	} else {
		/* final sigma context specific rule */
		if (x == 0x03a3 &&   /* U+03A3 = GREEK CAPITAL LETTER SIGMA */
		    prev >= 0 &&     /* prev is letter */
		    next < 0) {      /* next is not letter */
			/* FIXME: fix conditions */
			x = 0x03c2;
			goto singlechar;
		}

		/* FIXME: lithuanian */
		if (0 /* language == 'lt' */ &&
		    x == 0x0307) {               /* U+0307 = COMBINING DOT ABOVE */
			goto nochar;
		}

		/* FIXME: lithuanian, explicit dot rules */
		/* FIXME: turkish / azeri, lowercase rules */
	}

	/* 1:1 or special conversions, but not locale/context specific: script generated rules */
	DUK_MEMSET(&bd_ctx, 0, sizeof(bd_ctx));
	if (uppercase) {
		bd_ctx.data = (duk_uint8_t *) duk_unicode_caseconv_uc;
		bd_ctx.length = (duk_size_t) sizeof(duk_unicode_caseconv_uc);
	} else {
		bd_ctx.data = (duk_uint8_t *) duk_unicode_caseconv_lc;
		bd_ctx.length = (duk_size_t) sizeof(duk_unicode_caseconv_lc);
	}
	return slow_case_conversion(thr, buf, x, &bd_ctx);

 singlechar:
	if (buf) {
		duk_hbuffer_append_xutf8(thr, buf, x);
	}
	return x;

 nochar:
	return -1;
}

/*
 *  Replace valstack top with case converted version.
 */

void duk_unicode_case_convert_string(duk_hthread *thr, int uppercase) {
	duk_context *ctx = (duk_context *) thr;
	duk_hstring *h_input;
	duk_hbuffer_dynamic *h_buf;
	duk_uint8_t *p, *p_start, *p_end;
	int prev, curr, next;

	h_input = duk_require_hstring(ctx, -1);
	DUK_ASSERT(h_input != NULL);

	/* FIXME: should init with a spare of at least h_input->blen? */
	duk_push_dynamic_buffer(ctx, 0);
	h_buf = (duk_hbuffer_dynamic *) duk_get_hbuffer(ctx, -1);
	DUK_ASSERT(h_buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(h_buf));

	/* [ ... input buffer ] */

	p_start = (duk_uint8_t *) DUK_HSTRING_GET_DATA(h_input);
	p_end = p_start + DUK_HSTRING_GET_BYTELEN(h_input);
	p = p_start;

	prev = -1;
	curr = -1;
	next = -1;
	for (;;) {
		prev = curr;
		curr = next;
		next = -1;
		if (p < p_end) {
			next = (int) duk_unicode_xutf8_get_u32_checked(thr, &p, p_start, p_end);
		} else {
			/* end of input and last char has been processed */
			if (curr < 0) {
				break;
			}
		}

		/* on first round, skip */
		if (curr >= 0) {
			/* may generate any number of output codepoints */
			case_transform_helper(thr,
			                      h_buf,
			                      curr,
			                      prev,
			                      next,
			                      uppercase,
			                      0);  /* FIXME: language */
		}
	}

	duk_to_string(ctx, -1);  /* invalidates h_buf pointer */
	duk_remove(ctx, -2);
}

#ifdef DUK_USE_REGEXP_SUPPORT

/*
 *  Canonicalize() abstract operation needed for canonicalization of individual
 *  codepoints during regexp compilation and execution, see E5 Section 15.10.2.8.
 *  Note that codepoints are canonicalized one character at a time, so no context
 *  specific rules can apply.  Locale specific rules can apply, though.
 */

int duk_unicode_re_canonicalize_char(duk_hthread *thr, int x) {
	int y;

	y = case_transform_helper(thr,
	                          NULL,    /* buf */
	                          x,       /* curr char */
	                          -1,      /* prev char */
	                          -1,      /* next char */
	                          1,       /* uppercase */
	                          0);      /* FIXME: language */

	if ((y < 0) || (x >= 0x80 && y < 0x80)) {
		/* multiple codepoint conversion or non-ASCII mapped to ASCII
		 * --> leave as is.
		 */
		return x;
	}

	return y;
}

/*
 *  E5 Section 15.10.2.6 "IsWordChar" abstract operation.  Assume
 *  x < 0 for characters read outside the string.
 */

int duk_unicode_re_is_wordchar(int x) {
	/*
	 *  Note: the description in E5 Section 15.10.2.6 has a typo, it
	 *  contains 'A' twice and lacks 'a'; the intent is [0-9a-zA-Z_].
	 */
	if ((x >= '0' && x <= '9') ||
	    (x >= 'a' && x <= 'z') ||
	    (x >= 'A' && x <= 'Z') ||
	    (x == '_')) {
		return 1;
	}
	return 0;
}

/*
 *  Regexp range tables
 */

/* exposed because lexer needs these too */
duk_uint16_t duk_unicode_re_ranges_digit[2] = {
	(duk_uint16_t) 0x0030, (duk_uint16_t) 0x0039,
};
duk_uint16_t duk_unicode_re_ranges_white[22] = {
	(duk_uint16_t) 0x0009, (duk_uint16_t) 0x000D,
	(duk_uint16_t) 0x0020, (duk_uint16_t) 0x0020,
	(duk_uint16_t) 0x00A0, (duk_uint16_t) 0x00A0,
	(duk_uint16_t) 0x1680, (duk_uint16_t) 0x1680,
	(duk_uint16_t) 0x180E, (duk_uint16_t) 0x180E,
	(duk_uint16_t) 0x2000, (duk_uint16_t) 0x200A,
	(duk_uint16_t) 0x2028, (duk_uint16_t) 0x2029,
	(duk_uint16_t) 0x202F, (duk_uint16_t) 0x202F,
	(duk_uint16_t) 0x205F, (duk_uint16_t) 0x205F,
	(duk_uint16_t) 0x3000, (duk_uint16_t) 0x3000,
	(duk_uint16_t) 0xFEFF, (duk_uint16_t) 0xFEFF,
};
duk_uint16_t duk_unicode_re_ranges_wordchar[8] = {
	(duk_uint16_t) 0x0030, (duk_uint16_t) 0x0039,
	(duk_uint16_t) 0x0041, (duk_uint16_t) 0x005A,
	(duk_uint16_t) 0x005F, (duk_uint16_t) 0x005F,
	(duk_uint16_t) 0x0061, (duk_uint16_t) 0x007A,
};
duk_uint16_t duk_unicode_re_ranges_not_digit[4] = {
	(duk_uint16_t) 0x0000, (duk_uint16_t) 0x002F,
	(duk_uint16_t) 0x003A, (duk_uint16_t) 0xFFFF,
};
duk_uint16_t duk_unicode_re_ranges_not_white[24] = {
	(duk_uint16_t) 0x0000, (duk_uint16_t) 0x0008,
	(duk_uint16_t) 0x000E, (duk_uint16_t) 0x001F,
	(duk_uint16_t) 0x0021, (duk_uint16_t) 0x009F,
	(duk_uint16_t) 0x00A1, (duk_uint16_t) 0x167F,
	(duk_uint16_t) 0x1681, (duk_uint16_t) 0x180D,
	(duk_uint16_t) 0x180F, (duk_uint16_t) 0x1FFF,
	(duk_uint16_t) 0x200B, (duk_uint16_t) 0x2027,
	(duk_uint16_t) 0x202A, (duk_uint16_t) 0x202E,
	(duk_uint16_t) 0x2030, (duk_uint16_t) 0x205E,
	(duk_uint16_t) 0x2060, (duk_uint16_t) 0x2FFF,
	(duk_uint16_t) 0x3001, (duk_uint16_t) 0xFEFE,
	(duk_uint16_t) 0xFF00, (duk_uint16_t) 0xFFFF,
};
duk_uint16_t duk_unicode_re_ranges_not_wordchar[10] = {
	(duk_uint16_t) 0x0000, (duk_uint16_t) 0x002F,
	(duk_uint16_t) 0x003A, (duk_uint16_t) 0x0040,
	(duk_uint16_t) 0x005B, (duk_uint16_t) 0x005E,
	(duk_uint16_t) 0x0060, (duk_uint16_t) 0x0060,
	(duk_uint16_t) 0x007B, (duk_uint16_t) 0xFFFF,
};

#endif  /* DUK_USE_REGEXP_SUPPORT */

