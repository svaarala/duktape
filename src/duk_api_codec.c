/*
 *  Encoding and decoding basic formats: hex, base64.
 *
 *  These are in-place operations which may allow an optimized implementation.
 */

#include "duk_internal.h"

/* dst length must be exactly ceil(len/3)*4 */
DUK_LOCAL void duk__base64_encode_helper(const duk_uint8_t *src, const duk_uint8_t *src_end,
                                         duk_uint8_t *dst, duk_uint8_t *dst_end) {
	duk_small_uint_t i, snip;
	duk_uint_fast32_t t;
	duk_uint_fast8_t x, y;

	DUK_UNREF(dst_end);

	while (src < src_end) {
		/* read 3 bytes into 't', padded by zero */
		snip = 4;
		t = 0;
		for (i = 0; i < 3; i++) {
			t = t << 8;
			if (src >= src_end) {
				snip--;
			} else {
				t += (duk_uint_fast32_t) (*src++);
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

			DUK_ASSERT(dst < dst_end);
			*dst++ = (duk_uint8_t) y;
		}
	}
}

DUK_LOCAL duk_bool_t duk__base64_decode_helper(const duk_uint8_t *src, const duk_uint8_t *src_end,
                                               duk_uint8_t *dst, duk_uint8_t *dst_end, duk_uint8_t **out_dst_final) {
	duk_uint_fast32_t t;
	duk_uint_fast8_t x, y;
	duk_small_uint_t group_idx;

	DUK_UNREF(dst_end);

	t = 0;
	group_idx = 0;

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
			/* We don't check the zero padding bytes here right now.
			 * This seems to be common behavior for base-64 decoders.
			 */

			if (group_idx == 2) {
				/* xx== -> 1 byte, t contains 12 bits, 4 on right are zero */
				t = t >> 4;
				DUK_ASSERT(dst < dst_end);
				*dst++ = (duk_uint8_t) t;

				if (src >= src_end) {
					goto error;
				}
				x = *src++;
				if (x != '=') {
					goto error;
				}
			} else if (group_idx == 3) {
				/* xxx= -> 2 bytes, t contains 18 bits, 2 on right are zero */
				t = t >> 2;
				DUK_ASSERT(dst < dst_end);
				*dst++ = (duk_uint8_t) ((t >> 8) & 0xff);
				DUK_ASSERT(dst < dst_end);
				*dst++ = (duk_uint8_t) (t & 0xff);
			} else {
				goto error;
			}

			/* Here we can choose either to end parsing and ignore
			 * whatever follows, or to continue parsing in case
			 * multiple (possibly padded) base64 strings have been
			 * concatenated.  Currently, keep on parsing.
			 */
			t = 0;
			group_idx = 0;
			continue;
		} else if (x == 0x09 || x == 0x0a || x == 0x0d || x == 0x20) {
			/* allow basic ASCII whitespace */
			continue;
		} else {
			goto error;
		}

		t = (t << 6) + y;

		if (group_idx == 3) {
			/* output 3 bytes from 't' */
			DUK_ASSERT(dst < dst_end);
			*dst++ = (duk_uint8_t) ((t >> 16) & 0xff);
			DUK_ASSERT(dst < dst_end);
			*dst++ = (duk_uint8_t) ((t >> 8) & 0xff);
			DUK_ASSERT(dst < dst_end);
			*dst++ = (duk_uint8_t) (t & 0xff);
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
		goto error;
	}

	*out_dst_final = dst;
	return 1;

 error:
	return 0;
}

/* Shared handling for encode/decode argument.  Fast path handling for
 * buffer and string values because they're the most common.  In particular,
 * avoid creating a temporary string or buffer when possible.
 */
DUK_LOCAL const duk_uint8_t *duk__prep_codec_arg(duk_context *ctx, duk_idx_t index, duk_size_t *out_len) {
	DUK_ASSERT(duk_is_valid_index(ctx, index));  /* checked by caller */
	if (duk_is_buffer(ctx, index)) {
		return (const duk_uint8_t *) duk_get_buffer(ctx, index, out_len);
	} else {
		return (const duk_uint8_t *) duk_to_lstring(ctx, index, out_len);
	}
}

DUK_EXTERNAL const char *duk_base64_encode(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_uint8_t *src;
	duk_size_t srclen;
	duk_size_t dstlen;
	duk_uint8_t *dst;
	const char *ret;

	DUK_ASSERT_CTX_VALID(ctx);

	/* XXX: optimize for string inputs: no need to coerce to a buffer
	 * which makes a copy of the input.
	 */

	index = duk_require_normalize_index(ctx, index);
	src = (duk_uint8_t *) duk_to_buffer(ctx, index, &srclen);
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
	dst = (duk_uint8_t *) duk_push_fixed_buffer(ctx, dstlen);

	duk__base64_encode_helper((const duk_uint8_t *) src, (const duk_uint8_t *) (src + srclen),
	                          dst, (dst + dstlen));

	ret = duk_to_string(ctx, -1);
	duk_replace(ctx, index);
	return ret;

 type_error:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_BASE64_ENCODE_FAILED);
	return NULL;  /* never here */
}

DUK_EXTERNAL void duk_base64_decode(duk_context *ctx, duk_idx_t index) {
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

	index = duk_require_normalize_index(ctx, index);
	src = (const duk_uint8_t *) duk_to_lstring(ctx, index, &srclen);

	/* Computation must not wrap, only srclen + 3 is at risk of
	 * wrapping because after that the number gets smaller.
	 * This limit works for 32-bit size_t:
	 * 0x100000000 - 3 - 1 = 4294967292
	 */
	if (srclen > 4294967292UL) {
		goto type_error;
	}
	dstlen = (srclen + 3) / 4 * 3;  /* upper limit */
	dst = (duk_uint8_t *) duk_push_dynamic_buffer(ctx, dstlen);
	/* Note: for dstlen=0, dst may be NULL */

	retval = duk__base64_decode_helper((const duk_uint8_t *) src, (const duk_uint8_t *) (src + srclen),
	                                   dst, dst + dstlen, &dst_final);
	if (!retval) {
		goto type_error;
	}

	/* XXX: convert to fixed buffer? */
	(void) duk_resize_buffer(ctx, -1, (duk_size_t) (dst_final - dst));
	duk_replace(ctx, index);
	return;

 type_error:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_BASE64_DECODE_FAILED);
}

DUK_EXTERNAL const char *duk_hex_encode(duk_context *ctx, duk_idx_t index) {
	const duk_uint8_t *inp;
	duk_size_t len;
	duk_size_t i;
	duk_small_uint_t t;
	duk_uint8_t *buf;
	const char *ret;

	DUK_ASSERT_CTX_VALID(ctx);

	index = duk_require_normalize_index(ctx, index);
	inp = duk__prep_codec_arg(ctx, index, &len);
	DUK_ASSERT(inp != NULL || len == 0);

	/* Fixed buffer, no zeroing because we'll fill all the data. */
	buf = (duk_uint8_t *) duk_push_buffer_raw(ctx, len * 2, DUK_BUF_FLAG_NOZERO /*flags*/);
	DUK_ASSERT(buf != NULL);

	for (i = 0; i < len; i++) {
		/* XXX: by using two 256-entry tables could avoid shifting and masking. */
		t = (duk_small_uint_t) inp[i];
		buf[i*2 + 0] = duk_lc_digits[t >> 4];
		buf[i*2 + 1] = duk_lc_digits[t & 0x0f];
	}

	/* XXX: Using a string return value forces a string intern which is
	 * not always necessary.  As a rough performance measure, hex encode
	 * time for perf-testcases/test-hex-encode.js dropped from ~35s to
	 * ~15s without string coercion.  Change to returning a buffer and
	 * let the caller coerce to string if necessary?
	 */

	ret = duk_to_string(ctx, -1);
	duk_replace(ctx, index);
	return ret;
}

DUK_EXTERNAL void duk_hex_decode(duk_context *ctx, duk_idx_t index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	const duk_uint8_t *inp;
	duk_size_t len;
	duk_size_t i;
	duk_small_int_t t;
	duk_uint8_t *buf;

	DUK_ASSERT_CTX_VALID(ctx);

	index = duk_require_normalize_index(ctx, index);
	inp = duk__prep_codec_arg(ctx, index, &len);
	DUK_ASSERT(inp != NULL || len == 0);

	if (len & 0x01) {
		goto type_error;
	}

	/* Fixed buffer, no zeroing because we'll fill all the data. */
	buf = (duk_uint8_t *) duk_push_buffer_raw(ctx, len / 2, DUK_BUF_FLAG_NOZERO /*flags*/);
	DUK_ASSERT(buf != NULL);

	for (i = 0; i < len; i += 2) {
		/* For invalid characters the value -1 gets extended to
		 * at least 16 bits.  If either nybble is invalid, the
		 * resulting 't' will be < 0.
		 */
		t = (((duk_small_int_t) duk_hex_dectab[inp[i]]) << 4) |
		    ((duk_small_int_t) duk_hex_dectab[inp[i + 1]]);
		if (DUK_UNLIKELY(t < 0)) {
			goto type_error;
		}
		buf[i >> 1] = (duk_uint8_t) t;
	}

	duk_replace(ctx, index);
	return;

 type_error:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_HEX_DECODE_FAILED);
}

DUK_EXTERNAL const char *duk_json_encode(duk_context *ctx, duk_idx_t index) {
#ifdef DUK_USE_ASSERTIONS
	duk_idx_t top_at_entry;
#endif
	const char *ret;

	DUK_ASSERT_CTX_VALID(ctx);
#ifdef DUK_USE_ASSERTIONS
	top_at_entry = duk_get_top(ctx);
#endif

	index = duk_require_normalize_index(ctx, index);
	duk_bi_json_stringify_helper(ctx,
	                             index /*idx_value*/,
	                             DUK_INVALID_INDEX /*idx_replacer*/,
	                             DUK_INVALID_INDEX /*idx_space*/,
	                             0 /*flags*/);
	DUK_ASSERT(duk_is_string(ctx, -1));
	duk_replace(ctx, index);
	ret = duk_get_string(ctx, index);

	DUK_ASSERT(duk_get_top(ctx) == top_at_entry);

	return ret;
}

DUK_EXTERNAL void duk_json_decode(duk_context *ctx, duk_idx_t index) {
#ifdef DUK_USE_ASSERTIONS
	duk_idx_t top_at_entry;
#endif

	DUK_ASSERT_CTX_VALID(ctx);
#ifdef DUK_USE_ASSERTIONS
	top_at_entry = duk_get_top(ctx);
#endif

	index = duk_require_normalize_index(ctx, index);
	duk_bi_json_parse_helper(ctx,
	                         index /*idx_value*/,
	                         DUK_INVALID_INDEX /*idx_reviver*/,
	                         0 /*flags*/);
	duk_replace(ctx, index);

	DUK_ASSERT(duk_get_top(ctx) == top_at_entry);
}
