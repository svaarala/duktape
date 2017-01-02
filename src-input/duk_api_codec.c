/*
 *  Encoding and decoding basic formats: hex, base64.
 *
 *  These are in-place operations which may allow an optimized implementation.
 *
 *  Base-64: https://tools.ietf.org/html/rfc4648#section-4
 */

#include "duk_internal.h"

/* Shared handling for encode/decode argument.  Fast path handling for
 * buffer and string values because they're the most common.  In particular,
 * avoid creating a temporary string or buffer when possible.
 */
DUK_LOCAL const duk_uint8_t *duk__prep_codec_arg(duk_context *ctx, duk_idx_t idx, duk_size_t *out_len) {
	void *ptr;
	duk_bool_t isbuffer;

	DUK_ASSERT(duk_is_valid_index(ctx, idx));  /* checked by caller */

	/* XXX: with def_ptr set to a stack related pointer, isbuffer could
	 * be removed from the helper?
	 */
	ptr = duk_get_buffer_data_raw(ctx, idx, out_len, NULL /*def_ptr*/, 0 /*def_size*/, 0 /*throw_flag*/, &isbuffer);
	if (isbuffer) {
		DUK_ASSERT(*out_len == 0 || ptr != NULL);
		return (const duk_uint8_t *) ptr;
	}
	return (const duk_uint8_t *) duk_to_lstring(ctx, idx, out_len);
}

#if defined(DUK_USE_BASE64_FASTPATH)
DUK_LOCAL void duk__base64_encode_helper(const duk_uint8_t *src, duk_size_t srclen, duk_uint8_t *dst) {
	duk_uint_t t;
	duk_size_t n_full, n_full3, n_final;
	const duk_uint8_t *src_end_fast;

	n_full = srclen / 3;  /* full 3-byte -> 4-char conversions */
	n_full3 = n_full * 3;
	n_final = srclen - n_full3;
	DUK_ASSERT_DISABLE(n_final >= 0);
	DUK_ASSERT(n_final <= 2);

	src_end_fast = src + n_full3;
	while (DUK_UNLIKELY(src != src_end_fast)) {
		t = (duk_uint_t) (*src++);
		t = (t << 8) + (duk_uint_t) (*src++);
		t = (t << 8) + (duk_uint_t) (*src++);

		*dst++ = duk_base64_enctab[t >> 18];
		*dst++ = duk_base64_enctab[(t >> 12) & 0x3f];
		*dst++ = duk_base64_enctab[(t >> 6) & 0x3f];
		*dst++ = duk_base64_enctab[t & 0x3f];

#if 0  /* Tested: not faster on x64 */
		/* aaaaaabb bbbbcccc ccdddddd */
		dst[0] = duk_base64_enctab[(src[0] >> 2) & 0x3f];
		dst[1] = duk_base64_enctab[((src[0] << 4) & 0x30) | ((src[1] >> 4) & 0x0f)];
		dst[2] = duk_base64_enctab[((src[1] << 2) & 0x3f) | ((src[2] >> 6) & 0x03)];
		dst[3] = duk_base64_enctab[src[2] & 0x3f];
		src += 3; dst += 4;
#endif
	}

	switch (n_final) {
	/* case 0: nop */
	case 1: {
		/* XX== */
		t = (duk_uint_t) (*src++);
		*dst++ = duk_base64_enctab[t >> 2];           /* XXXXXX-- */
		*dst++ = duk_base64_enctab[(t << 4) & 0x3f];  /* ------XX */
		*dst++ = DUK_ASC_EQUALS;
		*dst++ = DUK_ASC_EQUALS;
		break;
	}
	case 2: {
		/* XXX= */
		t = (duk_uint_t) (*src++);
		t = (t << 8) + (duk_uint_t) (*src++);
		*dst++ = duk_base64_enctab[t >> 10];          /* XXXXXX-- -------- */
		*dst++ = duk_base64_enctab[(t >> 4) & 0x3f];  /* ------XX XXXX---- */
		*dst++ = duk_base64_enctab[(t << 2) & 0x3f];  /* -------- ----XXXX */
		*dst++ = DUK_ASC_EQUALS;
		break;
	}
	}
}
#else  /* DUK_USE_BASE64_FASTPATH */
DUK_LOCAL void duk__base64_encode_helper(const duk_uint8_t *src, duk_size_t srclen, duk_uint8_t *dst) {
	duk_small_uint_t i, snip;
	duk_uint_t t;
	duk_uint_fast8_t x, y;
	const duk_uint8_t *src_end;

	src_end = src + srclen;

	while (src < src_end) {
		/* read 3 bytes into 't', padded by zero */
		snip = 4;
		t = 0;
		for (i = 0; i < 3; i++) {
			t = t << 8;
			if (src >= src_end) {
				snip--;
			} else {
				t += (duk_uint_t) (*src++);
			}
		}

		/*
		 *  Missing bytes    snip     base64 example
		 *    0               4         XXXX
		 *    1               3         XXX=
		 *    2               2         XX==
		 */

		DUK_ASSERT(snip >= 2 && snip <= 4);

		for (i = 0; i < 4; i++) {
			x = (duk_uint_fast8_t) ((t >> 18) & 0x3f);
			t = t << 6;

			/* A straightforward 64-byte lookup would be faster
			 * and cleaner, but this is shorter.
			 */
			if (i >= snip) {
				y = '=';
			} else if (x <= 25) {
				y = x + 'A';
			} else if (x <= 51) {
				y = x - 26 + 'a';
			} else if (x <= 61) {
				y = x - 52 + '0';
			} else if (x == 62) {
				y = '+';
			} else {
				y = '/';
			}

			*dst++ = (duk_uint8_t) y;
		}
	}
}
#endif  /* DUK_USE_BASE64_FASTPATH */

#if defined(DUK_USE_BASE64_FASTPATH)
DUK_LOCAL duk_bool_t duk__base64_decode_helper(const duk_uint8_t *src, duk_size_t srclen, duk_uint8_t *dst, duk_uint8_t **out_dst_final) {
	duk_int_t x;
	duk_int_t t;
	duk_small_uint_t n_equal;
	duk_small_uint_t n_chars;
	const duk_uint8_t *src_end;
	const duk_uint8_t *src_end_safe;

	src_end = src + srclen;
	src_end_safe = src_end - 4;  /* if 'src < src_end_safe', safe to read 4 bytes */

	/* Innermost fast path processes 4 valid base-64 characters at a time
	 * but bails out on whitespace, padding chars ('=') and invalid chars.
	 * Once the slow path segment has been processed, we return to the
	 * inner fast path again.  This handles e.g. base64 with newlines
	 * reasonably well because the majority of a line is in the fast path.
	 */
	for (;;) {
		/* Fast path, handle units with just actual encoding characters. */

		while (src <= src_end_safe) {
			/* The lookup byte is intentionally sign extended to (at least)
			 * 32 bits and then ORed.  This ensures that is at least 1 byte
			 * is negative, the highest bit of 't' will be set at the end
			 * and we don't need to check every byte.
			 */
			DUK_DDD(DUK_DDDPRINT("fast loop: src=%p, src_end_safe=%p, src_end=%p",
			                     (const void *) src, (const void *) src_end_safe, (const void *) src_end));

			t = (duk_int_t) duk_base64_dectab[*src++];
			t = (t << 6) | (duk_int_t) duk_base64_dectab[*src++];
			t = (t << 6) | (duk_int_t) duk_base64_dectab[*src++];
			t = (t << 6) | (duk_int_t) duk_base64_dectab[*src++];

			if (DUK_UNLIKELY(t < 0)) {
				DUK_DDD(DUK_DDDPRINT("fast loop unit was not clean, process one slow path unit"));
				src -= 4;
				break;
			}

			DUK_ASSERT(t <= 0xffffffL);
			DUK_ASSERT((t >> 24) == 0);
			*dst++ = (duk_uint8_t) (t >> 16);
			*dst++ = (duk_uint8_t) ((t >> 8) & 0xff);
			*dst++ = (duk_uint8_t) (t & 0xff);
		}

		/* Handle one slow path unit (or finish if we're done). */

		n_equal = 0;
		n_chars = 0;
		t = 0;
		for (;;) {
			DUK_DDD(DUK_DDDPRINT("slow loop: src=%p, src_end=%p, n_chars=%ld, n_equal=%ld, t=%ld",
			                     (const void *) src, (const void *) src_end, (long) n_chars, (long) n_equal, (long) t));

			if (DUK_UNLIKELY(src >= src_end)) {
				goto done;  /* two level break */
			}

			x = duk_base64_dectab[*src++];
			if (DUK_UNLIKELY(x < 0)) {
				if (x == -2) {
					continue;  /* allowed ascii whitespace */
				} else if (x == -3) {
					n_equal++;
					t <<= 6;
				} else {
					DUK_ASSERT(x == -1);
					goto decode_error;
				}
			} else {
				DUK_ASSERT(x >= 0 && x <= 63);
				if (n_equal > 0) {
					/* Don't allow actual chars after equal sign. */
					goto decode_error;
				}
				t = (t << 6) + x;
			}

			if (DUK_UNLIKELY(n_chars == 3)) {
				/* Emit 3 bytes and backtrack if there was padding.  There's
				 * always space for the whole 3 bytes so no check needed.
				 */
				DUK_ASSERT(t <= 0xffffffL);
				DUK_ASSERT((t >> 24) == 0);
				*dst++ = (duk_uint8_t) (t >> 16);
				*dst++ = (duk_uint8_t) ((t >> 8) & 0xff);
				*dst++ = (duk_uint8_t) (t & 0xff);

				if (DUK_UNLIKELY(n_equal > 0)) {
					DUK_ASSERT(n_equal <= 4);

					/* There may be whitespace between the equal signs. */
					if (n_equal == 1) {
						/* XXX= */
						dst -= 1;
					} else if (n_equal == 2) {
						/* XX== */
						dst -= 2;
					} else {
						goto decode_error;  /* invalid padding */
					}

					/* Continue parsing after padding, allows concatenated,
					 * padded base64.
					 */
				}
				break;  /* back to fast loop */
			} else {
				n_chars++;
			}
		}
	}
 done:
	DUK_DDD(DUK_DDDPRINT("done; src=%p, src_end=%p, n_chars=%ld",
	                     (const void *) src, (const void *) src_end, (long) n_chars));

	DUK_ASSERT(src == src_end);

	if (n_chars != 0) {
		/* Here we'd have the option of decoding unpadded base64
		 * (e.g. "xxxxyy" instead of "xxxxyy==".  Currently not
		 * accepted.
		 */
		goto decode_error;
	}

	*out_dst_final = dst;
	return 1;

 decode_error:
	return 0;
}
#else  /* DUK_USE_BASE64_FASTPATH */
DUK_LOCAL duk_bool_t duk__base64_decode_helper(const duk_uint8_t *src, duk_size_t srclen, duk_uint8_t *dst, duk_uint8_t **out_dst_final) {
	duk_uint_t t;
	duk_uint_fast8_t x, y;
	duk_small_uint_t group_idx;
	duk_small_uint_t n_equal;
	const duk_uint8_t *src_end;

	src_end = src + srclen;
	t = 0;
	group_idx = 0;
	n_equal = 0;

	while (src < src_end) {
		x = *src++;

		if (x >= 'A' && x <= 'Z') {
			y = x - 'A' + 0;
		} else if (x >= 'a' && x <= 'z') {
			y = x - 'a' + 26;
		} else if (x >= '0' && x <= '9') {
			y = x - '0' + 52;
		} else if (x == '+') {
			y = 62;
		} else if (x == '/') {
			y = 63;
		} else if (x == '=') {
			/* We don't check the zero padding bytes here right now
			 * (that they're actually zero).  This seems to be common
			 * behavior for base-64 decoders.
			 */

			n_equal++;
			t <<= 6;  /* shift in zeroes */
			goto skip_add;
		} else if (x == 0x09 || x == 0x0a || x == 0x0d || x == 0x20) {
			/* allow basic ASCII whitespace */
			continue;
		} else {
			goto decode_error;
		}

		if (n_equal > 0) {
			/* Don't allow mixed padding and actual chars. */
			goto decode_error;
		}
		t = (t << 6) + y;
	 skip_add:

		if (group_idx == 3) {
			/* output 3 bytes from 't' */
			*dst++ = (duk_uint8_t) ((t >> 16) & 0xff);
			*dst++ = (duk_uint8_t) ((t >> 8) & 0xff);
			*dst++ = (duk_uint8_t) (t & 0xff);

			if (DUK_UNLIKELY(n_equal > 0)) {
				/* Backtrack. */
				DUK_ASSERT(n_equal <= 4);
				if (n_equal == 1) {
					dst -= 1;
				} else if (n_equal == 2) {
					dst -= 2;
				} else {
					goto decode_error;  /* invalid padding */
				}

				/* Here we can choose either to end parsing and ignore
				 * whatever follows, or to continue parsing in case
				 * multiple (possibly padded) base64 strings have been
				 * concatenated.  Currently, keep on parsing.
				 */
				n_equal = 0;
			}

			t = 0;
			group_idx = 0;
		} else {
			group_idx++;
		}
	}

	if (group_idx != 0) {
		/* Here we'd have the option of decoding unpadded base64
		 * (e.g. "xxxxyy" instead of "xxxxyy==".  Currently not
		 * accepted.
		 */
		goto decode_error;
	}

	*out_dst_final = dst;
	return 1;

 decode_error:
	return 0;
}
#endif  /* DUK_USE_BASE64_FASTPATH */

DUK_EXTERNAL const char *duk_base64_encode(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	const duk_uint8_t *src;
	duk_size_t srclen;
	duk_size_t dstlen;
	duk_uint8_t *dst;
	const char *ret;

	DUK_ASSERT_CTX_VALID(ctx);

	/* XXX: optimize for string inputs: no need to coerce to a buffer
	 * which makes a copy of the input.
	 */

	idx = duk_require_normalize_index(ctx, idx);
	src = duk__prep_codec_arg(ctx, idx, &srclen);
	/* Note: for srclen=0, src may be NULL */

	/* Computation must not wrap; this limit works for 32-bit size_t:
	 * >>> srclen = 3221225469
	 * >>> '%x' % ((srclen + 2) / 3 * 4)
	 * 'fffffffc'
	 */
	if (srclen > 3221225469UL) {
		goto type_error;
	}
	dstlen = (srclen + 2) / 3 * 4;
	dst = (duk_uint8_t *) duk_push_fixed_buffer_nozero(ctx, dstlen);

	duk__base64_encode_helper((const duk_uint8_t *) src, srclen, dst);

	ret = duk_buffer_to_string(ctx, -1);  /* Safe, result is ASCII. */
	duk_replace(ctx, idx);
	return ret;

 type_error:
	DUK_ERROR_TYPE(thr, DUK_STR_ENCODE_FAILED);
	return NULL;  /* never here */
}

DUK_EXTERNAL void duk_base64_decode(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	const duk_uint8_t *src;
	duk_size_t srclen;
	duk_size_t dstlen;
	duk_uint8_t *dst;
	duk_uint8_t *dst_final;
	duk_bool_t retval;

	DUK_ASSERT_CTX_VALID(ctx);

	/* XXX: optimize for buffer inputs: no need to coerce to a string
	 * which causes an unnecessary interning.
	 */

	idx = duk_require_normalize_index(ctx, idx);
	src = duk__prep_codec_arg(ctx, idx, &srclen);

	/* Computation must not wrap, only srclen + 3 is at risk of
	 * wrapping because after that the number gets smaller.
	 * This limit works for 32-bit size_t:
	 * 0x100000000 - 3 - 1 = 4294967292
	 */
	if (srclen > 4294967292UL) {
		goto type_error;
	}
	dstlen = (srclen + 3) / 4 * 3;  /* upper limit, assuming no whitespace etc */
	dst = (duk_uint8_t *) duk_push_dynamic_buffer(ctx, dstlen);
	/* Note: for dstlen=0, dst may be NULL */

	retval = duk__base64_decode_helper((const duk_uint8_t *) src, srclen, dst, &dst_final);
	if (!retval) {
		goto type_error;
	}

	/* XXX: convert to fixed buffer? */
	(void) duk_resize_buffer(ctx, -1, (duk_size_t) (dst_final - dst));
	duk_replace(ctx, idx);
	return;

 type_error:
	DUK_ERROR_TYPE(thr, DUK_STR_DECODE_FAILED);
}

DUK_EXTERNAL const char *duk_hex_encode(duk_context *ctx, duk_idx_t idx) {
	const duk_uint8_t *inp;
	duk_size_t len;
	duk_size_t i;
	duk_uint8_t *buf;
	const char *ret;
#if defined(DUK_USE_HEX_FASTPATH)
	duk_size_t len_safe;
	duk_uint16_t *p16;
#endif

	DUK_ASSERT_CTX_VALID(ctx);

	idx = duk_require_normalize_index(ctx, idx);
	inp = duk__prep_codec_arg(ctx, idx, &len);
	DUK_ASSERT(inp != NULL || len == 0);

	/* Fixed buffer, no zeroing because we'll fill all the data. */
	buf = (duk_uint8_t *) duk_push_fixed_buffer_nozero(ctx, len * 2);
	DUK_ASSERT(buf != NULL);

#if defined(DUK_USE_HEX_FASTPATH)
	DUK_ASSERT((((duk_size_t) buf) & 0x01U) == 0);   /* pointer is aligned, guaranteed for fixed buffer */
	p16 = (duk_uint16_t *) (void *) buf;
	len_safe = len & ~0x03U;
	for (i = 0; i < len_safe; i += 4) {
		p16[0] = duk_hex_enctab[inp[i]];
		p16[1] = duk_hex_enctab[inp[i + 1]];
		p16[2] = duk_hex_enctab[inp[i + 2]];
		p16[3] = duk_hex_enctab[inp[i + 3]];
		p16 += 4;
	}
	for (; i < len; i++) {
		*p16++ = duk_hex_enctab[inp[i]];
	}
#else  /* DUK_USE_HEX_FASTPATH */
	for (i = 0; i < len; i++) {
		duk_small_uint_t t;
		t = (duk_small_uint_t) inp[i];
		buf[i*2 + 0] = duk_lc_digits[t >> 4];
		buf[i*2 + 1] = duk_lc_digits[t & 0x0f];
	}
#endif  /* DUK_USE_HEX_FASTPATH */

	/* XXX: Using a string return value forces a string intern which is
	 * not always necessary.  As a rough performance measure, hex encode
	 * time for tests/perf/test-hex-encode.js dropped from ~35s to ~15s
	 * without string coercion.  Change to returning a buffer and let the
	 * caller coerce to string if necessary?
	 */

	ret = duk_buffer_to_string(ctx, -1);  /* Safe, result is ASCII. */
	duk_replace(ctx, idx);
	return ret;
}

DUK_EXTERNAL void duk_hex_decode(duk_context *ctx, duk_idx_t idx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	const duk_uint8_t *inp;
	duk_size_t len;
	duk_size_t i;
	duk_int_t t;
	duk_uint8_t *buf;
#if defined(DUK_USE_HEX_FASTPATH)
	duk_int_t chk;
	duk_uint8_t *p;
	duk_size_t len_safe;
#endif

	DUK_ASSERT_CTX_VALID(ctx);

	idx = duk_require_normalize_index(ctx, idx);
	inp = duk__prep_codec_arg(ctx, idx, &len);
	DUK_ASSERT(inp != NULL || len == 0);

	if (len & 0x01) {
		goto type_error;
	}

	/* Fixed buffer, no zeroing because we'll fill all the data. */
	buf = (duk_uint8_t *) duk_push_fixed_buffer_nozero(ctx, len / 2);
	DUK_ASSERT(buf != NULL);

#if defined(DUK_USE_HEX_FASTPATH)
	p = buf;
	len_safe = len & ~0x07U;
	for (i = 0; i < len_safe; i += 8) {
		t = ((duk_int_t) duk_hex_dectab_shift4[inp[i]]) |
		    ((duk_int_t) duk_hex_dectab[inp[i + 1]]);
		chk = t;
		p[0] = (duk_uint8_t) t;
		t = ((duk_int_t) duk_hex_dectab_shift4[inp[i + 2]]) |
		    ((duk_int_t) duk_hex_dectab[inp[i + 3]]);
		chk |= t;
		p[1] = (duk_uint8_t) t;
		t = ((duk_int_t) duk_hex_dectab_shift4[inp[i + 4]]) |
		    ((duk_int_t) duk_hex_dectab[inp[i + 5]]);
		chk |= t;
		p[2] = (duk_uint8_t) t;
		t = ((duk_int_t) duk_hex_dectab_shift4[inp[i + 6]]) |
		    ((duk_int_t) duk_hex_dectab[inp[i + 7]]);
		chk |= t;
		p[3] = (duk_uint8_t) t;
		p += 4;

		/* Check if any lookup above had a negative result. */
		if (DUK_UNLIKELY(chk < 0)) {
			goto type_error;
		}
	}
	for (; i < len; i += 2) {
		t = (((duk_int_t) duk_hex_dectab[inp[i]]) << 4) |
		    ((duk_int_t) duk_hex_dectab[inp[i + 1]]);
		if (DUK_UNLIKELY(t < 0)) {
			goto type_error;
		}
		*p++ = (duk_uint8_t) t;
	}
#else  /* DUK_USE_HEX_FASTPATH */
	for (i = 0; i < len; i += 2) {
		/* For invalid characters the value -1 gets extended to
		 * at least 16 bits.  If either nybble is invalid, the
		 * resulting 't' will be < 0.
		 */
		t = (((duk_int_t) duk_hex_dectab[inp[i]]) << 4) |
		    ((duk_int_t) duk_hex_dectab[inp[i + 1]]);
		if (DUK_UNLIKELY(t < 0)) {
			goto type_error;
		}
		buf[i >> 1] = (duk_uint8_t) t;
	}
#endif  /* DUK_USE_HEX_FASTPATH */

	duk_replace(ctx, idx);
	return;

 type_error:
	DUK_ERROR_TYPE(thr, DUK_STR_DECODE_FAILED);
}

#if defined(DUK_USE_JSON_SUPPORT)
DUK_EXTERNAL const char *duk_json_encode(duk_context *ctx, duk_idx_t idx) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t top_at_entry;
#endif
	const char *ret;

	DUK_ASSERT_CTX_VALID(ctx);
#if defined(DUK_USE_ASSERTIONS)
	top_at_entry = duk_get_top(ctx);
#endif

	idx = duk_require_normalize_index(ctx, idx);
	duk_bi_json_stringify_helper(ctx,
	                             idx /*idx_value*/,
	                             DUK_INVALID_INDEX /*idx_replacer*/,
	                             DUK_INVALID_INDEX /*idx_space*/,
	                             0 /*flags*/);
	DUK_ASSERT(duk_is_string(ctx, -1));
	duk_replace(ctx, idx);
	ret = duk_get_string(ctx, idx);

	DUK_ASSERT(duk_get_top(ctx) == top_at_entry);

	return ret;
}

DUK_EXTERNAL void duk_json_decode(duk_context *ctx, duk_idx_t idx) {
#if defined(DUK_USE_ASSERTIONS)
	duk_idx_t top_at_entry;
#endif

	DUK_ASSERT_CTX_VALID(ctx);
#if defined(DUK_USE_ASSERTIONS)
	top_at_entry = duk_get_top(ctx);
#endif

	idx = duk_require_normalize_index(ctx, idx);
	duk_bi_json_parse_helper(ctx,
	                         idx /*idx_value*/,
	                         DUK_INVALID_INDEX /*idx_reviver*/,
	                         0 /*flags*/);
	duk_replace(ctx, idx);

	DUK_ASSERT(duk_get_top(ctx) == top_at_entry);
}
#else  /* DUK_USE_JSON_SUPPORT */
DUK_EXTERNAL const char *duk_json_encode(duk_context *ctx, duk_idx_t idx) {
	DUK_UNREF(idx);
	DUK_ERROR_UNSUPPORTED((duk_hthread *) ctx);
}

DUK_EXTERNAL void duk_json_decode(duk_context *ctx, duk_idx_t idx) {
	DUK_UNREF(idx);
	DUK_ERROR_UNSUPPORTED((duk_hthread *) ctx);
}
#endif  /* DUK_USE_JSON_SUPPORT */
