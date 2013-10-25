/*
 *  Encoding and decoding basic formats: hex, base64.
 *
 *  These are in-place operations which may allow an optimized implementation.
 */

#include "duk_internal.h"

/* dst length must be exactly ceil(len/3)*4 */
static void base64_encode_helper(const unsigned char *src, const unsigned char *src_end,
                                 unsigned char *dst, unsigned char *dst_end) {
	unsigned int i, snip;
	unsigned int x, y, t;

	while (src < src_end) {
		/* read 3 bytes into 't', padded by zero */
		snip = 4;
		t = 0;
		for (i = 0; i < 3; i++) {
			t = t << 8;
			if (src >= src_end) {
				snip--;
			} else {
				t += (unsigned int) (*src++);
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
			x = (t >> 18) & 0x3f;
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
			*dst++ = (unsigned char) y;
		}
	}
}

static int base64_decode_helper(const unsigned char *src, const unsigned char *src_end,
                                unsigned char *dst, unsigned char *dst_end, unsigned char **out_dst_final) {
	unsigned int t;
	unsigned int x, y;
	int group_idx;

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
				*dst++ = (unsigned char) t;

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
				*dst++ = (unsigned char) ((t >> 8) & 0xff);
				DUK_ASSERT(dst < dst_end);
				*dst++ = (unsigned char) (t & 0xff);
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
			*dst++ = (unsigned char) ((t >> 16) & 0xff);
			DUK_ASSERT(dst < dst_end);
			*dst++ = (unsigned char) ((t >> 8) & 0xff);
			DUK_ASSERT(dst < dst_end);
			*dst++ = (unsigned char) (t & 0xff);
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

const char *duk_base64_encode(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	unsigned char *src;
	size_t srclen;
	size_t dstlen;
	unsigned char *dst;
	const char *ret;

	/* FIXME: optimize for string inputs: no need to coerce to a buffer
	 * which makes a copy of the input.
	 */

	index = duk_require_normalize_index(ctx, index);
	src = (unsigned char *) duk_to_buffer(ctx, index, &srclen);
	/* Note: for srclen=0, src may be NULL */

	/* Computation must not wrap; this limit works for 32-bit size_t:
	 * >>> srclen = 3221225469
	 * >>> '%x' % ((srclen + 2) / 3 * 4)
	 * 'fffffffc'
	 */
	if (srclen > 3221225469U) {
		goto type_error;
	}
	dstlen = (srclen + 2) / 3 * 4;
	dst = (unsigned char *) duk_push_fixed_buffer(ctx, dstlen);

	base64_encode_helper((const unsigned char *) src, (const unsigned char *) (src + srclen),
	                     (unsigned char *) dst, (unsigned char *) (dst + dstlen));

	ret = duk_to_string(ctx, -1);
	duk_replace(ctx, index);
	return ret;

 type_error:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "base64 encode failed");
	return NULL;  /* never here */
}

void duk_base64_decode(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	const char *src;
	size_t srclen;
	size_t dstlen;
	unsigned char *dst;
	unsigned char *dst_final;
	int retval;

	/* FIXME: optimize for buffer inputs: no need to coerce to a string
	 * which causes an unnecessary interning.
	 */

	index = duk_require_normalize_index(ctx, index);
	src = duk_to_lstring(ctx, index, &srclen);

	/* Computation must not wrap, only srclen + 3 is at risk of
	 * wrapping because after that the number gets smaller.
	 * This limit works for 32-bit size_t:
	 * 0x100000000 - 3 - 1 = 4294967292
	 */
	if (srclen > 4294967292U) {
		goto type_error;
	}
	dstlen = (srclen + 3) / 4 * 3;  /* upper limit */
	dst = (unsigned char *) duk_push_dynamic_buffer(ctx, dstlen);
	/* Note: for dstlen=0, dst may be NULL */

	retval = base64_decode_helper((unsigned char *) src, (unsigned char *) (src + srclen),
	                              dst, dst + dstlen, &dst_final);
	if (!retval) {
		goto type_error;
	}

	/* XXX: convert to fixed buffer? */
	(void) duk_resize_buffer(ctx, -1, (size_t) (dst_final - dst));
	duk_replace(ctx, index);
	return;

 type_error:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "base64 decode failed");
}

const char *duk_hex_encode(duk_context *ctx, int index) {
	unsigned char *data;
	size_t len;
	size_t i;
	int t;
	unsigned char *buf;
	const char *ret;

	/* FIXME: special case for input string, no need to coerce to buffer */

	index = duk_require_normalize_index(ctx, index);
	data = (unsigned char *) duk_to_buffer(ctx, index, &len);
	DUK_ASSERT(data != NULL);

	buf = (unsigned char *) duk_push_fixed_buffer(ctx, len * 2);
	DUK_ASSERT(buf != NULL);
	/* buf is always zeroed */

	for (i = 0; i < len; i++) {
		t = data[i];
		buf[i*2 + 0] = duk_lc_digits[t >> 4];
		buf[i*2 + 1] = duk_lc_digits[t & 0x0f];
	}

	ret = duk_to_string(ctx, -1);
	duk_replace(ctx, index);
	return ret;
}

void duk_hex_decode(duk_context *ctx, int index) {
	duk_hthread *thr = (duk_hthread *) ctx;
	const char *str;
	size_t len;
	size_t i;
	int t;
	unsigned char *buf;

	/* FIXME: optimize for buffer inputs: no need to coerce to a string
	 * which causes an unnecessary interning.
	 */

	index = duk_require_normalize_index(ctx, index);
	str = duk_to_lstring(ctx, index, &len);
	DUK_ASSERT(str != NULL);

	if (len & 0x01) {
		goto type_error;
	}

	buf = (unsigned char *) duk_push_fixed_buffer(ctx, len / 2);
	DUK_ASSERT(buf != NULL);
	/* buf is always zeroed */

	for (i = 0; i < len; i++) {
		t = str[i];
		if (t >= '0' && t <= '9') {
			t = t - '0' + 0x00;
		} else if (t >= 'a' && t <= 'f') {
			t = t - 'a' + 0x0a;
		} else if (t >= 'A' && t <= 'F') {
			t = t - 'A' + 0x0a;
		} else {
			goto type_error;
		}

		if (i & 0x01) {
			buf[i >> 1] += (unsigned char) t;
		} else {
			buf[i >> 1] = (unsigned char) (t << 4);
		}
	}

	duk_replace(ctx, index);
	return;

 type_error:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, "hex decode failed");
}

const char *duk_json_encode(duk_context *ctx, int index) {
#ifdef DUK_USE_ASSERTIONS
	int top_at_entry = duk_get_top(ctx);
#endif
	const char *ret;

	index = duk_require_normalize_index(ctx, index);
	duk_builtin_json_stringify_helper(ctx,
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

void duk_json_decode(duk_context *ctx, int index) {
#ifdef DUK_USE_ASSERTIONS
	int top_at_entry = duk_get_top(ctx);
#endif

	index = duk_require_normalize_index(ctx, index);
	duk_builtin_json_parse_helper(ctx,
	                              index /*idx_value*/,
	                              DUK_INVALID_INDEX /*idx_reviver*/,
	                              0 /*flags*/);
	duk_replace(ctx, index);

	DUK_ASSERT(duk_get_top(ctx) == top_at_entry);
}

