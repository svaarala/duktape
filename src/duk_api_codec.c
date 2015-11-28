/*
 *  Encoding and decoding basic formats: hex, base64.
 *
 *  These are in-place operations which may allow an optimized implementation.
 *
 *  Base-64: https://tools.ietf.org/html/rfc4648#section-4
 */

#include "duk_internal.h"

#if defined(DUK_USE_BASE64_FASTPATH)
DUK_LOCAL const duk_uint8_t duk__base64_enc_lookup[64] = {
	0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 0x50,  /* A...P */
	0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,  /* Q...f */
	0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76,  /* g...v */
	0x77, 0x78, 0x79, 0x7a, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x2b, 0x2f   /* w.../ */
};
DUK_LOCAL const duk_int8_t duk__base64_dec_lookup[256] = {
	/* -1 = error, -2 = allowed whitespace, -3 = padding ('='), 0...63 decoded bytes */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -2, -2, -1, -1, -2, -1, -1,  /* 0x00...0x0f */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  /* 0x10...0x1f */
	-2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,  /* 0x20...0x2f */
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -3, -1, -1,  /* 0x30...0x3f */
	-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,  /* 0x40...0x4f */
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,  /* 0x50...0x5f */
	-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,  /* 0x60...0x6f */
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,  /* 0x70...0x7f */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  /* 0x80...0x8f */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  /* 0x90...0x9f */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  /* 0xa0...0xaf */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  /* 0xb0...0xbf */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  /* 0xc0...0xcf */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  /* 0xd0...0xdf */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  /* 0xe0...0xef */
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1   /* 0xf0...0xff */
};
#endif  /* DUK_USE_BASE64_FASTPATH */

#if defined(DUK_USE_HEX_FASTPATH)
/* Lookup to encode one byte directly into 2 characters:
 *
 *   def genhextab(bswap):
 *       for i in xrange(256):
 *           t = chr(i).encode('hex')
 *           if bswap:
 *               t = t[1] + t[0]
 *           print('0x' + t.encode('hex') + 'U')
 *   print('big endian'); genhextab(False)
 *   print('little endian'); genhextab(True)
*/
#if defined(DUK_USE_INTEGER_BE)
DUK_LOCAL duk_uint16_t duk__hex_enc_lookup[256] = {
	0x3030U, 0x3031U, 0x3032U, 0x3033U, 0x3034U, 0x3035U, 0x3036U, 0x3037U,
	0x3038U, 0x3039U, 0x3061U, 0x3062U, 0x3063U, 0x3064U, 0x3065U, 0x3066U,
	0x3130U, 0x3131U, 0x3132U, 0x3133U, 0x3134U, 0x3135U, 0x3136U, 0x3137U,
	0x3138U, 0x3139U, 0x3161U, 0x3162U, 0x3163U, 0x3164U, 0x3165U, 0x3166U,
	0x3230U, 0x3231U, 0x3232U, 0x3233U, 0x3234U, 0x3235U, 0x3236U, 0x3237U,
	0x3238U, 0x3239U, 0x3261U, 0x3262U, 0x3263U, 0x3264U, 0x3265U, 0x3266U,
	0x3330U, 0x3331U, 0x3332U, 0x3333U, 0x3334U, 0x3335U, 0x3336U, 0x3337U,
	0x3338U, 0x3339U, 0x3361U, 0x3362U, 0x3363U, 0x3364U, 0x3365U, 0x3366U,
	0x3430U, 0x3431U, 0x3432U, 0x3433U, 0x3434U, 0x3435U, 0x3436U, 0x3437U,
	0x3438U, 0x3439U, 0x3461U, 0x3462U, 0x3463U, 0x3464U, 0x3465U, 0x3466U,
	0x3530U, 0x3531U, 0x3532U, 0x3533U, 0x3534U, 0x3535U, 0x3536U, 0x3537U,
	0x3538U, 0x3539U, 0x3561U, 0x3562U, 0x3563U, 0x3564U, 0x3565U, 0x3566U,
	0x3630U, 0x3631U, 0x3632U, 0x3633U, 0x3634U, 0x3635U, 0x3636U, 0x3637U,
	0x3638U, 0x3639U, 0x3661U, 0x3662U, 0x3663U, 0x3664U, 0x3665U, 0x3666U,
	0x3730U, 0x3731U, 0x3732U, 0x3733U, 0x3734U, 0x3735U, 0x3736U, 0x3737U,
	0x3738U, 0x3739U, 0x3761U, 0x3762U, 0x3763U, 0x3764U, 0x3765U, 0x3766U,
	0x3830U, 0x3831U, 0x3832U, 0x3833U, 0x3834U, 0x3835U, 0x3836U, 0x3837U,
	0x3838U, 0x3839U, 0x3861U, 0x3862U, 0x3863U, 0x3864U, 0x3865U, 0x3866U,
	0x3930U, 0x3931U, 0x3932U, 0x3933U, 0x3934U, 0x3935U, 0x3936U, 0x3937U,
	0x3938U, 0x3939U, 0x3961U, 0x3962U, 0x3963U, 0x3964U, 0x3965U, 0x3966U,
	0x6130U, 0x6131U, 0x6132U, 0x6133U, 0x6134U, 0x6135U, 0x6136U, 0x6137U,
	0x6138U, 0x6139U, 0x6161U, 0x6162U, 0x6163U, 0x6164U, 0x6165U, 0x6166U,
	0x6230U, 0x6231U, 0x6232U, 0x6233U, 0x6234U, 0x6235U, 0x6236U, 0x6237U,
	0x6238U, 0x6239U, 0x6261U, 0x6262U, 0x6263U, 0x6264U, 0x6265U, 0x6266U,
	0x6330U, 0x6331U, 0x6332U, 0x6333U, 0x6334U, 0x6335U, 0x6336U, 0x6337U,
	0x6338U, 0x6339U, 0x6361U, 0x6362U, 0x6363U, 0x6364U, 0x6365U, 0x6366U,
	0x6430U, 0x6431U, 0x6432U, 0x6433U, 0x6434U, 0x6435U, 0x6436U, 0x6437U,
	0x6438U, 0x6439U, 0x6461U, 0x6462U, 0x6463U, 0x6464U, 0x6465U, 0x6466U,
	0x6530U, 0x6531U, 0x6532U, 0x6533U, 0x6534U, 0x6535U, 0x6536U, 0x6537U,
	0x6538U, 0x6539U, 0x6561U, 0x6562U, 0x6563U, 0x6564U, 0x6565U, 0x6566U,
	0x6630U, 0x6631U, 0x6632U, 0x6633U, 0x6634U, 0x6635U, 0x6636U, 0x6637U,
	0x6638U, 0x6639U, 0x6661U, 0x6662U, 0x6663U, 0x6664U, 0x6665U, 0x6666U
};
#else  /* DUK_USE_INTEGER_BE */
DUK_LOCAL duk_uint16_t duk__hex_enc_lookup[256] = {
	0x3030U, 0x3130U, 0x3230U, 0x3330U, 0x3430U, 0x3530U, 0x3630U, 0x3730U,
	0x3830U, 0x3930U, 0x6130U, 0x6230U, 0x6330U, 0x6430U, 0x6530U, 0x6630U,
	0x3031U, 0x3131U, 0x3231U, 0x3331U, 0x3431U, 0x3531U, 0x3631U, 0x3731U,
	0x3831U, 0x3931U, 0x6131U, 0x6231U, 0x6331U, 0x6431U, 0x6531U, 0x6631U,
	0x3032U, 0x3132U, 0x3232U, 0x3332U, 0x3432U, 0x3532U, 0x3632U, 0x3732U,
	0x3832U, 0x3932U, 0x6132U, 0x6232U, 0x6332U, 0x6432U, 0x6532U, 0x6632U,
	0x3033U, 0x3133U, 0x3233U, 0x3333U, 0x3433U, 0x3533U, 0x3633U, 0x3733U,
	0x3833U, 0x3933U, 0x6133U, 0x6233U, 0x6333U, 0x6433U, 0x6533U, 0x6633U,
	0x3034U, 0x3134U, 0x3234U, 0x3334U, 0x3434U, 0x3534U, 0x3634U, 0x3734U,
	0x3834U, 0x3934U, 0x6134U, 0x6234U, 0x6334U, 0x6434U, 0x6534U, 0x6634U,
	0x3035U, 0x3135U, 0x3235U, 0x3335U, 0x3435U, 0x3535U, 0x3635U, 0x3735U,
	0x3835U, 0x3935U, 0x6135U, 0x6235U, 0x6335U, 0x6435U, 0x6535U, 0x6635U,
	0x3036U, 0x3136U, 0x3236U, 0x3336U, 0x3436U, 0x3536U, 0x3636U, 0x3736U,
	0x3836U, 0x3936U, 0x6136U, 0x6236U, 0x6336U, 0x6436U, 0x6536U, 0x6636U,
	0x3037U, 0x3137U, 0x3237U, 0x3337U, 0x3437U, 0x3537U, 0x3637U, 0x3737U,
	0x3837U, 0x3937U, 0x6137U, 0x6237U, 0x6337U, 0x6437U, 0x6537U, 0x6637U,
	0x3038U, 0x3138U, 0x3238U, 0x3338U, 0x3438U, 0x3538U, 0x3638U, 0x3738U,
	0x3838U, 0x3938U, 0x6138U, 0x6238U, 0x6338U, 0x6438U, 0x6538U, 0x6638U,
	0x3039U, 0x3139U, 0x3239U, 0x3339U, 0x3439U, 0x3539U, 0x3639U, 0x3739U,
	0x3839U, 0x3939U, 0x6139U, 0x6239U, 0x6339U, 0x6439U, 0x6539U, 0x6639U,
	0x3061U, 0x3161U, 0x3261U, 0x3361U, 0x3461U, 0x3561U, 0x3661U, 0x3761U,
	0x3861U, 0x3961U, 0x6161U, 0x6261U, 0x6361U, 0x6461U, 0x6561U, 0x6661U,
	0x3062U, 0x3162U, 0x3262U, 0x3362U, 0x3462U, 0x3562U, 0x3662U, 0x3762U,
	0x3862U, 0x3962U, 0x6162U, 0x6262U, 0x6362U, 0x6462U, 0x6562U, 0x6662U,
	0x3063U, 0x3163U, 0x3263U, 0x3363U, 0x3463U, 0x3563U, 0x3663U, 0x3763U,
	0x3863U, 0x3963U, 0x6163U, 0x6263U, 0x6363U, 0x6463U, 0x6563U, 0x6663U,
	0x3064U, 0x3164U, 0x3264U, 0x3364U, 0x3464U, 0x3564U, 0x3664U, 0x3764U,
	0x3864U, 0x3964U, 0x6164U, 0x6264U, 0x6364U, 0x6464U, 0x6564U, 0x6664U,
	0x3065U, 0x3165U, 0x3265U, 0x3365U, 0x3465U, 0x3565U, 0x3665U, 0x3765U,
	0x3865U, 0x3965U, 0x6165U, 0x6265U, 0x6365U, 0x6465U, 0x6565U, 0x6665U,
	0x3066U, 0x3166U, 0x3266U, 0x3366U, 0x3466U, 0x3566U, 0x3666U, 0x3766U,
	0x3866U, 0x3966U, 0x6166U, 0x6266U, 0x6366U, 0x6466U, 0x6566U, 0x6666U
};
#endif  /* DUK_USE_INTEGER_BE */
#endif  /* DUK_USE_HEX_FASTPATH */

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

#if defined(DUK_USE_BASE64_FASTPATH)
DUK_LOCAL void duk__base64_encode_helper(const duk_uint8_t *src, duk_size_t srclen, duk_uint8_t *dst) {
	duk_uint_fast32_t t;
	duk_size_t n_full, n_final;

	n_full = srclen / 3;  /* full 3-byte -> 4-char conversions */
	n_final = srclen - n_full * 3;
	DUK_ASSERT_DISABLE(n_final >= 0);
	DUK_ASSERT(n_final <= 2);

	/* XXX: might be faster to avoid building 't' value explicitly,
	 * and index the lookup directly with expressions involving
	 * masked/shifted 'src' reads.
	 */

	while (n_full > 0) {
		n_full--;

		t = (duk_uint_fast32_t) (*src++);
		t = (t << 8) + (duk_uint_fast32_t) (*src++);
		t = (t << 8) + (duk_uint_fast32_t) (*src++);

		*dst++ = duk__base64_enc_lookup[t >> 18];
		*dst++ = duk__base64_enc_lookup[(t >> 12) & 0x3f];
		*dst++ = duk__base64_enc_lookup[(t >> 6) & 0x3f];
		*dst++ = duk__base64_enc_lookup[t & 0x3f];
	}

	switch (n_final) {
	/* case 0: nop */
	case 1: {
		/* XX== */
		t = (duk_uint_fast32_t) (*src++);
		*dst++ = duk__base64_enc_lookup[t >> 2];           /* XXXXXX-- */
		*dst++ = duk__base64_enc_lookup[(t << 4) & 0x3f];  /* ------XX */
		*dst++ = DUK_ASC_EQUALS;
		*dst++ = DUK_ASC_EQUALS;
		break;
	}
	case 2: {
		/* XXX= */
		t = (duk_uint_fast32_t) (*src++);
		t = (t << 8) + (duk_uint_fast32_t) (*src++);
		*dst++ = duk__base64_enc_lookup[t >> 10];          /* XXXXXX-- -------- */
		*dst++ = duk__base64_enc_lookup[(t >> 4) & 0x3f];  /* ------XX XXXX---- */
		*dst++ = duk__base64_enc_lookup[(t << 2) & 0x3f];  /* -------- ----XXXX */
		*dst++ = DUK_ASC_EQUALS;
		break;
	}
	}
}
#else  /* DUK_USE_BASE64_FASTPATH */
DUK_LOCAL void duk__base64_encode_helper(const duk_uint8_t *src, duk_size_t srclen, duk_uint8_t *dst) {
	duk_small_uint_t i, snip;
	duk_uint_fast32_t t;
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

			*dst++ = (duk_uint8_t) y;
		}
	}
}
#endif  /* DUK_USE_BASE64_FASTPATH */

#if defined(DUK_USE_BASE64_FASTPATH)
DUK_LOCAL duk_bool_t duk__base64_decode_helper(const duk_uint8_t *src, duk_size_t srclen, duk_uint8_t *dst, duk_uint8_t **out_dst_final) {
	duk_int_fast32_t x;
	duk_uint_fast32_t t;
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
			 * 32 bits and then ORed unsigned.  This ensures that is at least
			 * 1 byte is negative, the highest bit of 't' will be set at the
			 * end and we don't need to check every byte.
			 */
			DUK_DDD(DUK_DDDPRINT("fast loop: src=%p, src_end_safe=%p, src_end=%p",
			                     (const void *) src, (const void *) src_end_safe, (const void *) src_end));

			t = (duk_uint_fast32_t) (duk_int_fast32_t) duk__base64_dec_lookup[*src++];
			t = (t << 6) | (duk_uint_fast32_t) (duk_int_fast32_t) duk__base64_dec_lookup[*src++];
			t = (t << 6) | (duk_uint_fast32_t) (duk_int_fast32_t) duk__base64_dec_lookup[*src++];
			t = (t << 6) | (duk_uint_fast32_t) (duk_int_fast32_t) duk__base64_dec_lookup[*src++];

			if (DUK_UNLIKELY(t & 0x80000000UL)) {
				DUK_DDD(DUK_DDDPRINT("fast loop unit was not clean, process one slow path unit"));
				src -= 4;
				break;
			}

			DUK_ASSERT(t <= 0xffffffUL);
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

			x = duk__base64_dec_lookup[*src++];
			if (DUK_UNLIKELY(x < 0)) {
				if (x == -2) {
					continue;  /* allowed ascii whitespace */
				} else if (x == -3) {
					n_equal++;
					t <<= 6;
				} else {
					DUK_ASSERT(x == -1);
					goto error;
				}
			} else {
				DUK_ASSERT(x >= 0 && x <= 63);
				if (n_equal > 0) {
					/* Don't allow actual chars after equal sign. */
					goto error;
				}
				t = (t << 6) + x;
			}

			if (DUK_UNLIKELY(n_chars == 3)) {
				/* Emit 3 bytes and backtrack if there was padding.  There's
				 * always space for the whole 3 bytes so no check needed.
				 */
				DUK_ASSERT(t <= 0xffffffUL);
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
						goto error;  /* invalid padding */
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
		goto error;
	}

	*out_dst_final = dst;
	return 1;

 error:
	return 0;
}
#else  /* DUK_USE_BASE64_FASTPATH */
DUK_LOCAL duk_bool_t duk__base64_decode_helper(const duk_uint8_t *src, duk_size_t srclen, duk_uint8_t *dst, duk_uint8_t **out_dst_final) {
	duk_uint_fast32_t t;
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
			goto error;
		}

		if (n_equal > 0) {
			/* Don't allow mixed padding and actual chars. */
			goto error;
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
					goto error;  /* invalid padding */
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
		goto error;
	}

	*out_dst_final = dst;
	return 1;

 error:
	return 0;
}
#endif  /* DUK_USE_BASE64_FASTPATH */

DUK_EXTERNAL const char *duk_base64_encode(duk_context *ctx, duk_idx_t index) {
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

	index = duk_require_normalize_index(ctx, index);
	src = duk__prep_codec_arg(ctx, index, &srclen);
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

	duk__base64_encode_helper((const duk_uint8_t *) src, srclen, dst);

	ret = duk_to_string(ctx, -1);
	duk_replace(ctx, index);
	return ret;

 type_error:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_ENCODE_FAILED);
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
	src = duk__prep_codec_arg(ctx, index, &srclen);

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
	duk_replace(ctx, index);
	return;

 type_error:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_DECODE_FAILED);
}

DUK_EXTERNAL const char *duk_hex_encode(duk_context *ctx, duk_idx_t index) {
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

	index = duk_require_normalize_index(ctx, index);
	inp = duk__prep_codec_arg(ctx, index, &len);
	DUK_ASSERT(inp != NULL || len == 0);

	/* Fixed buffer, no zeroing because we'll fill all the data. */
	buf = (duk_uint8_t *) duk_push_buffer_raw(ctx, len * 2, DUK_BUF_FLAG_NOZERO /*flags*/);
	DUK_ASSERT(buf != NULL);

#if defined(DUK_USE_HEX_FASTPATH)
	p16 = (duk_uint16_t *) (void *) buf;  /* pointer is aligned */
	len_safe = len & ~0x03U;
	for (i = 0; i < len_safe; i += 4) {
		p16[0] = duk__hex_enc_lookup[inp[i]];
		p16[1] = duk__hex_enc_lookup[inp[i + 1]];
		p16[2] = duk__hex_enc_lookup[inp[i + 2]];
		p16[3] = duk__hex_enc_lookup[inp[i + 3]];
		p16 += 4;
	}
	for (; i < len; i++) {
		*p16++ = duk__hex_enc_lookup[inp[i]];
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
#if defined(DUK_USE_HEX_FASTPATH)
	duk_uint8_t *p;
	duk_size_t len_safe;
#endif

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

#if defined(DUK_USE_HEX_FASTPATH)
	p = buf;
	len_safe = len & ~0x07U;
	for (i = 0; i < len_safe; i += 8) {
		t = ((duk_small_int_t) duk_hex_dectab_shift4[inp[i]]) |
		    ((duk_small_int_t) duk_hex_dectab[inp[i + 1]]);
		if (DUK_UNLIKELY(t < 0)) {
			goto type_error;
		}
		p[0] = (duk_uint8_t) t;
		t = ((duk_small_int_t) duk_hex_dectab_shift4[inp[i + 2]]) |
		    ((duk_small_int_t) duk_hex_dectab[inp[i + 3]]);
		if (DUK_UNLIKELY(t < 0)) {
			goto type_error;
		}
		p[1] = (duk_uint8_t) t;
		t = ((duk_small_int_t) duk_hex_dectab_shift4[inp[i + 4]]) |
		    ((duk_small_int_t) duk_hex_dectab[inp[i + 5]]);
		if (DUK_UNLIKELY(t < 0)) {
			goto type_error;
		}
		p[2] = (duk_uint8_t) t;
		t = ((duk_small_int_t) duk_hex_dectab_shift4[inp[i + 6]]) |
		    ((duk_small_int_t) duk_hex_dectab[inp[i + 7]]);
		if (DUK_UNLIKELY(t < 0)) {
			goto type_error;
		}
		p[3] = (duk_uint8_t) t;
		p += 4;
	}
	for (; i < len; i += 2) {
		t = (((duk_small_int_t) duk_hex_dectab[inp[i]]) << 4) |
		    ((duk_small_int_t) duk_hex_dectab[inp[i + 1]]);
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
		t = ((duk_small_int_t) duk_hex_dectab_shift4[inp[i]]) |
		    ((duk_small_int_t) duk_hex_dectab[inp[i + 1]]);
		if (DUK_UNLIKELY(t < 0)) {
			goto type_error;
		}
		buf[i >> 1] = (duk_uint8_t) t;
	}
#endif  /* DUK_USE_HEX_FASTPATH */

	duk_replace(ctx, index);
	return;

 type_error:
	DUK_ERROR(thr, DUK_ERR_TYPE_ERROR, DUK_STR_DECODE_FAILED);
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
