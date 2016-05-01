/*
 *  Global object built-ins
 */

#include "duk_internal.h"

/*
 *  Encoding/decoding helpers
 */

/* XXX: Could add fast path (for each transform callback) with direct byte
 * lookups (no shifting) and no explicit check for x < 0x80 before table
 * lookup.
 */

/* Macros for creating and checking bitmasks for character encoding.
 * Bit number is a bit counterintuitive, but minimizes code size.
 */
#define DUK__MKBITS(a,b,c,d,e,f,g,h)  ((duk_uint8_t) ( \
	((a) << 0) | ((b) << 1) | ((c) << 2) | ((d) << 3) | \
	((e) << 4) | ((f) << 5) | ((g) << 6) | ((h) << 7) \
	))
#define DUK__CHECK_BITMASK(table,cp)  ((table)[(cp) >> 3] & (1 << ((cp) & 0x07)))

/* E5.1 Section 15.1.3.3: uriReserved + uriUnescaped + '#' */
DUK_LOCAL const duk_uint8_t duk__encode_uriunescaped_table[16] = {
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x00-0x0f */
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x10-0x1f */
	DUK__MKBITS(0, 1, 0, 1, 1, 0, 1, 1), DUK__MKBITS(1, 1, 1, 1, 1, 1, 1, 1),  /* 0x20-0x2f */
	DUK__MKBITS(1, 1, 1, 1, 1, 1, 1, 1), DUK__MKBITS(1, 1, 1, 1, 0, 1, 0, 1),  /* 0x30-0x3f */
	DUK__MKBITS(1, 1, 1, 1, 1, 1, 1, 1), DUK__MKBITS(1, 1, 1, 1, 1, 1, 1, 1),  /* 0x40-0x4f */
	DUK__MKBITS(1, 1, 1, 1, 1, 1, 1, 1), DUK__MKBITS(1, 1, 1, 0, 0, 0, 0, 1),  /* 0x50-0x5f */
	DUK__MKBITS(0, 1, 1, 1, 1, 1, 1, 1), DUK__MKBITS(1, 1, 1, 1, 1, 1, 1, 1),  /* 0x60-0x6f */
	DUK__MKBITS(1, 1, 1, 1, 1, 1, 1, 1), DUK__MKBITS(1, 1, 1, 0, 0, 0, 1, 0),  /* 0x70-0x7f */
};

/* E5.1 Section 15.1.3.4: uriUnescaped */
DUK_LOCAL const duk_uint8_t duk__encode_uricomponent_unescaped_table[16] = {
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x00-0x0f */
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x10-0x1f */
	DUK__MKBITS(0, 1, 0, 0, 0, 0, 0, 1), DUK__MKBITS(1, 1, 1, 0, 0, 1, 1, 0),  /* 0x20-0x2f */
	DUK__MKBITS(1, 1, 1, 1, 1, 1, 1, 1), DUK__MKBITS(1, 1, 0, 0, 0, 0, 0, 0),  /* 0x30-0x3f */
	DUK__MKBITS(0, 1, 1, 1, 1, 1, 1, 1), DUK__MKBITS(1, 1, 1, 1, 1, 1, 1, 1),  /* 0x40-0x4f */
	DUK__MKBITS(1, 1, 1, 1, 1, 1, 1, 1), DUK__MKBITS(1, 1, 1, 0, 0, 0, 0, 1),  /* 0x50-0x5f */
	DUK__MKBITS(0, 1, 1, 1, 1, 1, 1, 1), DUK__MKBITS(1, 1, 1, 1, 1, 1, 1, 1),  /* 0x60-0x6f */
	DUK__MKBITS(1, 1, 1, 1, 1, 1, 1, 1), DUK__MKBITS(1, 1, 1, 0, 0, 0, 1, 0),  /* 0x70-0x7f */
};

/* E5.1 Section 15.1.3.1: uriReserved + '#' */
DUK_LOCAL const duk_uint8_t duk__decode_uri_reserved_table[16] = {
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x00-0x0f */
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x10-0x1f */
	DUK__MKBITS(0, 0, 0, 1, 1, 0, 1, 0), DUK__MKBITS(0, 0, 0, 1, 1, 0, 0, 1),  /* 0x20-0x2f */
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 1, 1, 0, 1, 0, 1),  /* 0x30-0x3f */
	DUK__MKBITS(1, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x40-0x4f */
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x50-0x5f */
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x60-0x6f */
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x70-0x7f */
};

/* E5.1 Section 15.1.3.2: empty */
DUK_LOCAL const duk_uint8_t duk__decode_uri_component_reserved_table[16] = {
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x00-0x0f */
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x10-0x1f */
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x20-0x2f */
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x30-0x3f */
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x40-0x4f */
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x50-0x5f */
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x60-0x6f */
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x70-0x7f */
};

#ifdef DUK_USE_SECTION_B
/* E5.1 Section B.2.2, step 7. */
DUK_LOCAL const duk_uint8_t duk__escape_unescaped_table[16] = {
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x00-0x0f */
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x10-0x1f */
	DUK__MKBITS(0, 0, 0, 0, 0, 0, 0, 0), DUK__MKBITS(0, 0, 1, 1, 0, 1, 1, 1),  /* 0x20-0x2f */
	DUK__MKBITS(1, 1, 1, 1, 1, 1, 1, 1), DUK__MKBITS(1, 1, 0, 0, 0, 0, 0, 0),  /* 0x30-0x3f */
	DUK__MKBITS(1, 1, 1, 1, 1, 1, 1, 1), DUK__MKBITS(1, 1, 1, 1, 1, 1, 1, 1),  /* 0x40-0x4f */
	DUK__MKBITS(1, 1, 1, 1, 1, 1, 1, 1), DUK__MKBITS(1, 1, 1, 0, 0, 0, 0, 1),  /* 0x50-0x5f */
	DUK__MKBITS(0, 1, 1, 1, 1, 1, 1, 1), DUK__MKBITS(1, 1, 1, 1, 1, 1, 1, 1),  /* 0x60-0x6f */
	DUK__MKBITS(1, 1, 1, 1, 1, 1, 1, 1), DUK__MKBITS(1, 1, 1, 0, 0, 0, 0, 0)   /* 0x70-0x7f */
};
#endif  /* DUK_USE_SECTION_B */

#undef DUK__MKBITS

typedef struct {
	duk_hthread *thr;
	duk_hstring *h_str;
	duk_bufwriter_ctx bw;
	const duk_uint8_t *p;
	const duk_uint8_t *p_start;
	const duk_uint8_t *p_end;
} duk__transform_context;

typedef void (*duk__transform_callback)(duk__transform_context *tfm_ctx, const void *udata, duk_codepoint_t cp);

/* XXX: refactor and share with other code */
DUK_LOCAL duk_small_int_t duk__decode_hex_escape(const duk_uint8_t *p, duk_small_int_t n) {
	duk_small_int_t ch;
	duk_small_int_t t = 0;

	while (n > 0) {
		t = t * 16;
		ch = (duk_small_int_t) duk_hex_dectab[*p++];
		if (DUK_LIKELY(ch >= 0)) {
			t += ch;
		} else {
			return -1;
		}
		n--;
	}
	return t;
}

DUK_LOCAL int duk__transform_helper(duk_context *ctx, duk__transform_callback callback, const void *udata) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk__transform_context tfm_ctx_alloc;
	duk__transform_context *tfm_ctx = &tfm_ctx_alloc;
	duk_codepoint_t cp;

	tfm_ctx->thr = thr;

	tfm_ctx->h_str = duk_to_hstring(ctx, 0);
	DUK_ASSERT(tfm_ctx->h_str != NULL);

	DUK_BW_INIT_PUSHBUF(thr, &tfm_ctx->bw, DUK_HSTRING_GET_BYTELEN(tfm_ctx->h_str));  /* initial size guess */

	tfm_ctx->p_start = DUK_HSTRING_GET_DATA(tfm_ctx->h_str);
	tfm_ctx->p_end = tfm_ctx->p_start + DUK_HSTRING_GET_BYTELEN(tfm_ctx->h_str);
	tfm_ctx->p = tfm_ctx->p_start;

	while (tfm_ctx->p < tfm_ctx->p_end) {
		cp = (duk_codepoint_t) duk_unicode_decode_xutf8_checked(thr, &tfm_ctx->p, tfm_ctx->p_start, tfm_ctx->p_end);
		callback(tfm_ctx, udata, cp);
	}

	DUK_BW_COMPACT(thr, &tfm_ctx->bw);

	duk_to_string(ctx, -1);
	return 1;
}

DUK_LOCAL void duk__transform_callback_encode_uri(duk__transform_context *tfm_ctx, const void *udata, duk_codepoint_t cp) {
	duk_uint8_t xutf8_buf[DUK_UNICODE_MAX_XUTF8_LENGTH];
	duk_small_int_t len;
	duk_codepoint_t cp1, cp2;
	duk_small_int_t i, t;
	const duk_uint8_t *unescaped_table = (const duk_uint8_t *) udata;

	/* UTF-8 encoded bytes escaped as %xx%xx%xx... -> 3 * nbytes.
	 * Codepoint range is restricted so this is a slightly too large
	 * but doesn't matter.
	 */
	DUK_BW_ENSURE(tfm_ctx->thr, &tfm_ctx->bw, 3 * DUK_UNICODE_MAX_XUTF8_LENGTH);

	if (cp < 0) {
		goto uri_error;
	} else if ((cp < 0x80L) && DUK__CHECK_BITMASK(unescaped_table, cp)) {
		DUK_BW_WRITE_RAW_U8(tfm_ctx->thr, &tfm_ctx->bw, (duk_uint8_t) cp);
		return;
	} else if (cp >= 0xdc00L && cp <= 0xdfffL) {
		goto uri_error;
	} else if (cp >= 0xd800L && cp <= 0xdbffL) {
		/* Needs lookahead */
		if (duk_unicode_decode_xutf8(tfm_ctx->thr, &tfm_ctx->p, tfm_ctx->p_start, tfm_ctx->p_end, (duk_ucodepoint_t *) &cp2) == 0) {
			goto uri_error;
		}
		if (!(cp2 >= 0xdc00L && cp2 <= 0xdfffL)) {
			goto uri_error;
		}
		cp1 = cp;
		cp = ((cp1 - 0xd800L) << 10) + (cp2 - 0xdc00L) + 0x10000L;
	} else if (cp > 0x10ffffL) {
		/* Although we can allow non-BMP characters (they'll decode
		 * back into surrogate pairs), we don't allow extended UTF-8
		 * characters; they would encode to URIs which won't decode
		 * back because of strict UTF-8 checks in URI decoding.
		 * (However, we could just as well allow them here.)
		 */
		goto uri_error;
	} else {
		/* Non-BMP characters within valid UTF-8 range: encode as is.
		 * They'll decode back into surrogate pairs if the escaped
		 * output is decoded.
		 */
		;
	}

	len = duk_unicode_encode_xutf8((duk_ucodepoint_t) cp, xutf8_buf);
	for (i = 0; i < len; i++) {
		t = (int) xutf8_buf[i];
		DUK_BW_WRITE_RAW_U8_3(tfm_ctx->thr,
		                      &tfm_ctx->bw,
		                      DUK_ASC_PERCENT,
		                      (duk_uint8_t) duk_uc_nybbles[t >> 4],
                                      (duk_uint8_t) duk_uc_nybbles[t & 0x0f]);
	}

	return;

 uri_error:
	DUK_ERROR(tfm_ctx->thr, DUK_ERR_URI_ERROR, "invalid input");
}

DUK_LOCAL void duk__transform_callback_decode_uri(duk__transform_context *tfm_ctx, const void *udata, duk_codepoint_t cp) {
	const duk_uint8_t *reserved_table = (const duk_uint8_t *) udata;
	duk_small_uint_t utf8_blen;
	duk_codepoint_t min_cp;
	duk_small_int_t t;  /* must be signed */
	duk_small_uint_t i;

	/* Maximum write size: XUTF8 path writes max DUK_UNICODE_MAX_XUTF8_LENGTH,
	 * percent escape path writes max two times CESU-8 encoded BMP length.
	 */
	DUK_BW_ENSURE(tfm_ctx->thr,
	              &tfm_ctx->bw,
	              (DUK_UNICODE_MAX_XUTF8_LENGTH >= 2 * DUK_UNICODE_MAX_CESU8_BMP_LENGTH ?
	              DUK_UNICODE_MAX_XUTF8_LENGTH : DUK_UNICODE_MAX_CESU8_BMP_LENGTH));

	if (cp == (duk_codepoint_t) '%') {
		const duk_uint8_t *p = tfm_ctx->p;
		duk_size_t left = (duk_size_t) (tfm_ctx->p_end - p);  /* bytes left */

		DUK_DDD(DUK_DDDPRINT("percent encoding, left=%ld", (long) left));

		if (left < 2) {
			goto uri_error;
		}

		t = duk__decode_hex_escape(p, 2);
		DUK_DDD(DUK_DDDPRINT("first byte: %ld", (long) t));
		if (t < 0) {
			goto uri_error;
		}

		if (t < 0x80) {
			if (DUK__CHECK_BITMASK(reserved_table, t)) {
				/* decode '%xx' to '%xx' if decoded char in reserved set */
				DUK_ASSERT(tfm_ctx->p - 1 >= tfm_ctx->p_start);
				DUK_BW_WRITE_RAW_U8_3(tfm_ctx->thr,
				                      &tfm_ctx->bw,
				                      DUK_ASC_PERCENT,
				                      p[0],
				                      p[1]);
			} else {
				DUK_BW_WRITE_RAW_U8(tfm_ctx->thr, &tfm_ctx->bw, (duk_uint8_t) t);
			}
			tfm_ctx->p += 2;
			return;
		}

		/* Decode UTF-8 codepoint from a sequence of hex escapes.  The
		 * first byte of the sequence has been decoded to 't'.
		 *
		 * Note that UTF-8 validation must be strict according to the
		 * specification: E5.1 Section 15.1.3, decode algorithm step
		 * 4.d.vii.8.  URIError from non-shortest encodings is also
		 * specifically noted in the spec.
		 */

		DUK_ASSERT(t >= 0x80);
		if (t < 0xc0) {
			/* continuation byte */
			goto uri_error;
		} else if (t < 0xe0) {
			/* 110x xxxx; 2 bytes */
			utf8_blen = 2;
			min_cp = 0x80L;
			cp = t & 0x1f;
		} else if (t < 0xf0) {
			/* 1110 xxxx; 3 bytes */
			utf8_blen = 3;
			min_cp = 0x800L;
			cp = t & 0x0f;
		} else if (t < 0xf8) {
			/* 1111 0xxx; 4 bytes */
			utf8_blen = 4;
			min_cp = 0x10000L;
			cp = t & 0x07;
		} else {
			/* extended utf-8 not allowed for URIs */
			goto uri_error;
		}

		if (left < utf8_blen * 3 - 1) {
			/* '%xx%xx...%xx', p points to char after first '%' */
			goto uri_error;
		}

		p += 3;
		for (i = 1; i < utf8_blen; i++) {
			/* p points to digit part ('%xy', p points to 'x') */
			t = duk__decode_hex_escape(p, 2);
			DUK_DDD(DUK_DDDPRINT("i=%ld utf8_blen=%ld cp=%ld t=0x%02lx",
			                     (long) i, (long) utf8_blen, (long) cp, (unsigned long) t));
			if (t < 0) {
				goto uri_error;
			}
			if ((t & 0xc0) != 0x80) {
				goto uri_error;
			}
			cp = (cp << 6) + (t & 0x3f);
			p += 3;
		}
		p--;  /* p overshoots */
		tfm_ctx->p = p;

		DUK_DDD(DUK_DDDPRINT("final cp=%ld, min_cp=%ld", (long) cp, (long) min_cp));

		if (cp < min_cp || cp > 0x10ffffL || (cp >= 0xd800L && cp <= 0xdfffL)) {
			goto uri_error;
		}

		/* The E5.1 algorithm checks whether or not a decoded codepoint
		 * is below 0x80 and perhaps may be in the "reserved" set.
		 * This seems pointless because the single byte UTF-8 case is
		 * handled separately, and non-shortest encodings are rejected.
		 * So, 'cp' cannot be below 0x80 here, and thus cannot be in
		 * the reserved set.
		 */

		/* utf-8 validation ensures these */
		DUK_ASSERT(cp >= 0x80L && cp <= 0x10ffffL);

		if (cp >= 0x10000L) {
			cp -= 0x10000L;
			DUK_ASSERT(cp < 0x100000L);

			DUK_BW_WRITE_RAW_XUTF8(tfm_ctx->thr, &tfm_ctx->bw, ((cp >> 10) + 0xd800L));
			DUK_BW_WRITE_RAW_XUTF8(tfm_ctx->thr, &tfm_ctx->bw, ((cp & 0x03ffUL) + 0xdc00L));
		} else {
			DUK_BW_WRITE_RAW_XUTF8(tfm_ctx->thr, &tfm_ctx->bw, cp);
		}
	} else {
		DUK_BW_WRITE_RAW_XUTF8(tfm_ctx->thr, &tfm_ctx->bw, cp);
	}
	return;

 uri_error:
	DUK_ERROR(tfm_ctx->thr, DUK_ERR_URI_ERROR, "invalid input");
}

#ifdef DUK_USE_SECTION_B
DUK_LOCAL void duk__transform_callback_escape(duk__transform_context *tfm_ctx, const void *udata, duk_codepoint_t cp) {
	DUK_UNREF(udata);

	DUK_BW_ENSURE(tfm_ctx->thr, &tfm_ctx->bw, 6);

	if (cp < 0) {
		goto esc_error;
	} else if ((cp < 0x80L) && DUK__CHECK_BITMASK(duk__escape_unescaped_table, cp)) {
		DUK_BW_WRITE_RAW_U8(tfm_ctx->thr, &tfm_ctx->bw, (duk_uint8_t) cp);
	} else if (cp < 0x100L) {
		DUK_BW_WRITE_RAW_U8_3(tfm_ctx->thr,
		                      &tfm_ctx->bw,
		                      (duk_uint8_t) DUK_ASC_PERCENT,
		                      (duk_uint8_t) duk_uc_nybbles[cp >> 4],
		                      (duk_uint8_t) duk_uc_nybbles[cp & 0x0f]);
	} else if (cp < 0x10000L) {
		DUK_BW_WRITE_RAW_U8_6(tfm_ctx->thr,
		                      &tfm_ctx->bw,
		                      (duk_uint8_t) DUK_ASC_PERCENT,
		                      (duk_uint8_t) DUK_ASC_LC_U,
		                      (duk_uint8_t) duk_uc_nybbles[cp >> 12],
		                      (duk_uint8_t) duk_uc_nybbles[(cp >> 8) & 0x0f],
		                      (duk_uint8_t) duk_uc_nybbles[(cp >> 4) & 0x0f],
		                      (duk_uint8_t) duk_uc_nybbles[cp & 0x0f]);
	} else {
		/* Characters outside BMP cannot be escape()'d.  We could
		 * encode them as surrogate pairs (for codepoints inside
		 * valid UTF-8 range, but not extended UTF-8).  Because
		 * escape() and unescape() are legacy functions, we don't.
		 */
		goto esc_error;
	}

	return;

 esc_error:
	DUK_ERROR_TYPE(tfm_ctx->thr, "invalid input");
}

DUK_LOCAL void duk__transform_callback_unescape(duk__transform_context *tfm_ctx, const void *udata, duk_codepoint_t cp) {
	duk_small_int_t t;

	DUK_UNREF(udata);

	if (cp == (duk_codepoint_t) '%') {
		const duk_uint8_t *p = tfm_ctx->p;
		duk_size_t left = (duk_size_t) (tfm_ctx->p_end - p);  /* bytes left */

		if (left >= 5 && p[0] == 'u' &&
		    ((t = duk__decode_hex_escape(p + 1, 4)) >= 0)) {
			cp = (duk_codepoint_t) t;
			tfm_ctx->p += 5;
		} else if (left >= 2 &&
		           ((t = duk__decode_hex_escape(p, 2)) >= 0)) {
			cp = (duk_codepoint_t) t;
			tfm_ctx->p += 2;
		}
	}

	DUK_BW_WRITE_ENSURE_XUTF8(tfm_ctx->thr, &tfm_ctx->bw, cp);
}
#endif  /* DUK_USE_SECTION_B */

/*
 *  Eval
 *
 *  Eval needs to handle both a "direct eval" and an "indirect eval".
 *  Direct eval handling needs access to the caller's activation so that its
 *  lexical environment can be accessed.  A direct eval is only possible from
 *  Ecmascript code; an indirect eval call is possible also from C code.
 *  When an indirect eval call is made from C code, there may not be a
 *  calling activation at all which needs careful handling.
 */

DUK_INTERNAL duk_ret_t duk_bi_global_object_eval(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h;
	duk_activation *act_caller;
	duk_activation *act_eval;
	duk_activation *act;
	duk_hcompiledfunction *func;
	duk_hobject *outer_lex_env;
	duk_hobject *outer_var_env;
	duk_bool_t this_to_global = 1;
	duk_small_uint_t comp_flags;
	duk_int_t level = -2;

	DUK_ASSERT(duk_get_top(ctx) == 1 || duk_get_top(ctx) == 2);  /* 2 when called by debugger */
	DUK_ASSERT(thr->callstack_top >= 1);  /* at least this function exists */
	DUK_ASSERT(((thr->callstack + thr->callstack_top - 1)->flags & DUK_ACT_FLAG_DIRECT_EVAL) == 0 || /* indirect eval */
	           (thr->callstack_top >= 2));  /* if direct eval, calling activation must exist */

	/*
	 *  callstack_top - 1 --> this function
	 *  callstack_top - 2 --> caller (may not exist)
	 *
	 *  If called directly from C, callstack_top might be 1.  If calling
	 *  activation doesn't exist, call must be indirect.
	 */

	h = duk_get_hstring(ctx, 0);
	if (!h) {
		return 1;  /* return arg as-is */
	}

#if defined(DUK_USE_DEBUGGER_SUPPORT)
	/* NOTE: level is used only by the debugger and should never be present
	 * for an Ecmascript eval().
	 */
	DUK_ASSERT(level == -2);  /* by default, use caller's environment */
	if (duk_get_top(ctx) >= 2 && duk_is_number(ctx, 1)) {
		level = duk_get_int(ctx, 1);
	}
	DUK_ASSERT(level <= -2);  /* This is guaranteed by debugger code. */
#endif

	/* [ source ] */

	comp_flags = DUK_JS_COMPILE_FLAG_EVAL;
	act_eval = thr->callstack + thr->callstack_top - 1;    /* this function */
	if (thr->callstack_top >= (duk_size_t) -level) {
		/* Have a calling activation, check for direct eval (otherwise
		 * assume indirect eval.
		 */
		act_caller = thr->callstack + thr->callstack_top + level;  /* caller */
		if ((act_caller->flags & DUK_ACT_FLAG_STRICT) &&
		    (act_eval->flags & DUK_ACT_FLAG_DIRECT_EVAL)) {
			/* Only direct eval inherits strictness from calling code
			 * (E5.1 Section 10.1.1).
			 */
			comp_flags |= DUK_JS_COMPILE_FLAG_STRICT;
		}
	} else {
		DUK_ASSERT((act_eval->flags & DUK_ACT_FLAG_DIRECT_EVAL) == 0);
	}
	act_caller = NULL;  /* avoid dereference after potential callstack realloc */
	act_eval = NULL;

	duk_push_hstring_stridx(ctx, DUK_STRIDX_INPUT);  /* XXX: copy from caller? */
	duk_js_compile(thr,
	               (const duk_uint8_t *) DUK_HSTRING_GET_DATA(h),
	               (duk_size_t) DUK_HSTRING_GET_BYTELEN(h),
	               comp_flags);
	func = (duk_hcompiledfunction *) duk_get_hobject(ctx, -1);
	DUK_ASSERT(func != NULL);
	DUK_ASSERT(DUK_HOBJECT_IS_COMPILEDFUNCTION((duk_hobject *) func));

	/* [ source template ] */

	/* E5 Section 10.4.2 */
	DUK_ASSERT(thr->callstack_top >= 1);
	act = thr->callstack + thr->callstack_top - 1;  /* this function */
	if (act->flags & DUK_ACT_FLAG_DIRECT_EVAL) {
		DUK_ASSERT(thr->callstack_top >= 2);
		act = thr->callstack + thr->callstack_top + level;  /* caller */
		if (act->lex_env == NULL) {
			DUK_ASSERT(act->var_env == NULL);
			DUK_DDD(DUK_DDDPRINT("delayed environment initialization"));

			/* this may have side effects, so re-lookup act */
			duk_js_init_activation_environment_records_delayed(thr, act);
			act = thr->callstack + thr->callstack_top + level;
		}
		DUK_ASSERT(act->lex_env != NULL);
		DUK_ASSERT(act->var_env != NULL);

		this_to_global = 0;

		if (DUK_HOBJECT_HAS_STRICT((duk_hobject *) func)) {
			duk_hobject *new_env;
			duk_hobject *act_lex_env;

			DUK_DDD(DUK_DDDPRINT("direct eval call to a strict function -> "
			                     "var_env and lex_env to a fresh env, "
			                     "this_binding to caller's this_binding"));

			act = thr->callstack + thr->callstack_top + level;  /* caller */
			act_lex_env = act->lex_env;
			act = NULL;  /* invalidated */

			(void) duk_push_object_helper_proto(ctx,
			                                    DUK_HOBJECT_FLAG_EXTENSIBLE |
			                                    DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_DECENV),
			                                    act_lex_env);
			new_env = duk_require_hobject(ctx, -1);
			DUK_ASSERT(new_env != NULL);
			DUK_DDD(DUK_DDDPRINT("new_env allocated: %!iO",
			                     (duk_heaphdr *) new_env));

			outer_lex_env = new_env;
			outer_var_env = new_env;

			duk_insert(ctx, 0);  /* stash to bottom of value stack to keep new_env reachable for duration of eval */

			/* compiler's responsibility */
			DUK_ASSERT(DUK_HOBJECT_HAS_NEWENV((duk_hobject *) func));
		} else {
			DUK_DDD(DUK_DDDPRINT("direct eval call to a non-strict function -> "
			                     "var_env and lex_env to caller's envs, "
			                     "this_binding to caller's this_binding"));

			outer_lex_env = act->lex_env;
			outer_var_env = act->var_env;

			/* compiler's responsibility */
			DUK_ASSERT(!DUK_HOBJECT_HAS_NEWENV((duk_hobject *) func));
		}
	} else {
		DUK_DDD(DUK_DDDPRINT("indirect eval call -> var_env and lex_env to "
		                     "global object, this_binding to global object"));

		this_to_global = 1;
		outer_lex_env = thr->builtins[DUK_BIDX_GLOBAL_ENV];
		outer_var_env = thr->builtins[DUK_BIDX_GLOBAL_ENV];
	}
	act = NULL;

	/* Eval code doesn't need an automatic .prototype object. */
	duk_js_push_closure(thr, func, outer_var_env, outer_lex_env, 0 /*add_auto_proto*/);

	/* [ source template closure ] */

	if (this_to_global) {
		DUK_ASSERT(thr->builtins[DUK_BIDX_GLOBAL] != NULL);
		duk_push_hobject_bidx(ctx, DUK_BIDX_GLOBAL);
	} else {
		duk_tval *tv;
		DUK_ASSERT(thr->callstack_top >= 2);
		act = thr->callstack + thr->callstack_top + level;  /* caller */
		tv = thr->valstack + act->idx_bottom - 1;  /* this is just beneath bottom */
		DUK_ASSERT(tv >= thr->valstack);
		duk_push_tval(ctx, tv);
	}

	DUK_DDD(DUK_DDDPRINT("eval -> lex_env=%!iO, var_env=%!iO, this_binding=%!T",
	                     (duk_heaphdr *) outer_lex_env,
	                     (duk_heaphdr *) outer_var_env,
	                     duk_get_tval(ctx, -1)));

	/* [ source template closure this ] */

	duk_call_method(ctx, 0);

	/* [ source template result ] */

	return 1;
}

/*
 *  Parsing of ints and floats
 */

DUK_INTERNAL duk_ret_t duk_bi_global_object_parse_int(duk_context *ctx) {
	duk_int32_t radix;
	duk_small_uint_t s2n_flags;

	DUK_ASSERT_TOP(ctx, 2);
	duk_to_string(ctx, 0);

	radix = duk_to_int32(ctx, 1);

	s2n_flags = DUK_S2N_FLAG_TRIM_WHITE |
	            DUK_S2N_FLAG_ALLOW_GARBAGE |
	            DUK_S2N_FLAG_ALLOW_PLUS |
	            DUK_S2N_FLAG_ALLOW_MINUS |
	            DUK_S2N_FLAG_ALLOW_LEADING_ZERO |
	            DUK_S2N_FLAG_ALLOW_AUTO_HEX_INT;

	/* Specification stripPrefix maps to DUK_S2N_FLAG_ALLOW_AUTO_HEX_INT.
	 *
	 * Don't autodetect octals (from leading zeroes), require user code to
	 * provide an explicit radix 8 for parsing octal.  See write-up from Mozilla:
	 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/parseInt#ECMAScript_5_Removes_Octal_Interpretation
	 */

	if (radix != 0) {
		if (radix < 2 || radix > 36) {
			goto ret_nan;
		}
		if (radix != 16) {
			s2n_flags &= ~DUK_S2N_FLAG_ALLOW_AUTO_HEX_INT;
		}
	} else {
		radix = 10;
	}

	duk_dup(ctx, 0);
	duk_numconv_parse(ctx, radix, s2n_flags);
	return 1;

 ret_nan:
	duk_push_nan(ctx);
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_global_object_parse_float(duk_context *ctx) {
	duk_small_uint_t s2n_flags;
	duk_int32_t radix;

	DUK_ASSERT_TOP(ctx, 1);
	duk_to_string(ctx, 0);

	radix = 10;

	/* XXX: check flags */
	s2n_flags = DUK_S2N_FLAG_TRIM_WHITE |
	            DUK_S2N_FLAG_ALLOW_EXP |
	            DUK_S2N_FLAG_ALLOW_GARBAGE |
	            DUK_S2N_FLAG_ALLOW_PLUS |
	            DUK_S2N_FLAG_ALLOW_MINUS |
	            DUK_S2N_FLAG_ALLOW_INF |
	            DUK_S2N_FLAG_ALLOW_FRAC |
	            DUK_S2N_FLAG_ALLOW_NAKED_FRAC |
	            DUK_S2N_FLAG_ALLOW_EMPTY_FRAC |
	            DUK_S2N_FLAG_ALLOW_LEADING_ZERO;

	duk_numconv_parse(ctx, radix, s2n_flags);
	return 1;
}

/*
 *  Number checkers
 */

DUK_INTERNAL duk_ret_t duk_bi_global_object_is_nan(duk_context *ctx) {
	duk_double_t d = duk_to_number(ctx, 0);
	duk_push_boolean(ctx, DUK_ISNAN(d));
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_global_object_is_finite(duk_context *ctx) {
	duk_double_t d = duk_to_number(ctx, 0);
	duk_push_boolean(ctx, DUK_ISFINITE(d));
	return 1;
}

/*
 *  URI handling
 */

DUK_INTERNAL duk_ret_t duk_bi_global_object_decode_uri(duk_context *ctx) {
	return duk__transform_helper(ctx, duk__transform_callback_decode_uri, (const void *) duk__decode_uri_reserved_table);
}

DUK_INTERNAL duk_ret_t duk_bi_global_object_decode_uri_component(duk_context *ctx) {
	return duk__transform_helper(ctx, duk__transform_callback_decode_uri, (const void *) duk__decode_uri_component_reserved_table);
}

DUK_INTERNAL duk_ret_t duk_bi_global_object_encode_uri(duk_context *ctx) {
	return duk__transform_helper(ctx, duk__transform_callback_encode_uri, (const void *) duk__encode_uriunescaped_table);
}

DUK_INTERNAL duk_ret_t duk_bi_global_object_encode_uri_component(duk_context *ctx) {
	return duk__transform_helper(ctx, duk__transform_callback_encode_uri, (const void *) duk__encode_uricomponent_unescaped_table);
}

#ifdef DUK_USE_SECTION_B
DUK_INTERNAL duk_ret_t duk_bi_global_object_escape(duk_context *ctx) {
	return duk__transform_helper(ctx, duk__transform_callback_escape, (const void *) NULL);
}

DUK_INTERNAL duk_ret_t duk_bi_global_object_unescape(duk_context *ctx) {
	return duk__transform_helper(ctx, duk__transform_callback_unescape, (const void *) NULL);
}
#else  /* DUK_USE_SECTION_B */
DUK_INTERNAL duk_ret_t duk_bi_global_object_escape(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}

DUK_INTERNAL duk_ret_t duk_bi_global_object_unescape(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}
#endif  /* DUK_USE_SECTION_B */

#if defined(DUK_USE_BROWSER_LIKE) && (defined(DUK_USE_FILE_IO) || defined(DUK_USE_DEBUGGER_SUPPORT))
DUK_INTERNAL duk_ret_t duk_bi_global_object_print_helper(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_int_t magic;
	duk_idx_t nargs;
	const duk_uint8_t *buf;
	duk_size_t sz_buf;
	const char nl = (const char) DUK_ASC_LF;
#ifndef DUK_USE_PREFER_SIZE
	duk_uint8_t buf_stack[256];
#endif
#ifdef DUK_USE_FILE_IO
	duk_file *f_out;
#endif

	DUK_UNREF(thr);

	magic = duk_get_current_magic(ctx);
	DUK_UNREF(magic);

	nargs = duk_get_top(ctx);

	/* If argument count is 1 and first argument is a buffer, write the buffer
	 * as raw data into the file without a newline; this allows exact control
	 * over stdout/stderr without an additional entrypoint (useful for now).
	 *
	 * Otherwise current print/alert semantics are to ToString() coerce
	 * arguments, join them with a single space, and append a newline.
	 */

	if (nargs == 1 && duk_is_buffer(ctx, 0)) {
		buf = (const duk_uint8_t *) duk_get_buffer(ctx, 0, &sz_buf);
		DUK_ASSERT(buf != NULL);
	} else if (nargs > 0) {
#ifdef DUK_USE_PREFER_SIZE
		/* Compact but lots of churn. */
		duk_push_hstring_stridx(thr, DUK_STRIDX_SPACE);
		duk_insert(ctx, 0);
		duk_join(ctx, nargs);
		duk_push_string(thr, "\n");
		duk_concat(ctx, 2);
		buf = (const duk_uint8_t *) duk_get_lstring(ctx, -1, &sz_buf);
		DUK_ASSERT(buf != NULL);
#else  /* DUK_USE_PREFER_SIZE */
		/* Higher footprint, less churn. */
		duk_idx_t i;
		duk_size_t sz_str;
		const duk_uint8_t *p_str;
		duk_uint8_t *p;

		sz_buf = (duk_size_t) nargs;  /* spaces (nargs - 1) + newline */
		for (i = 0; i < nargs; i++) {
			(void) duk_to_lstring(ctx, i, &sz_str);
			sz_buf += sz_str;
		}

		if (sz_buf <= sizeof(buf_stack)) {
			p = (duk_uint8_t *) buf_stack;
		} else {
			p = (duk_uint8_t *) duk_push_fixed_buffer(ctx, sz_buf);
			DUK_ASSERT(p != NULL);
		}

		buf = (const duk_uint8_t *) p;
		for (i = 0; i < nargs; i++) {
			p_str = (const duk_uint8_t *) duk_get_lstring(ctx, i, &sz_str);
			DUK_ASSERT(p_str != NULL);
			DUK_MEMCPY((void *) p, (const void *) p_str, sz_str);
			p += sz_str;
			*p++ = (duk_uint8_t) (i == nargs - 1 ? DUK_ASC_LF : DUK_ASC_SPACE);
		}
		DUK_ASSERT((const duk_uint8_t *) p == buf + sz_buf);
#endif  /* DUK_USE_PREFER_SIZE */
	} else {
		buf = (const duk_uint8_t *) &nl;
		sz_buf = 1;
	}

	/* 'buf' contains the string to write, 'sz_buf' contains the length
	 * (which may be zero).
	 */
	DUK_ASSERT(buf != NULL);

	if (sz_buf == 0) {
		return 0;
	}

#ifdef DUK_USE_FILE_IO
	f_out = (magic ? DUK_STDERR : DUK_STDOUT);
	DUK_FWRITE((const void *) buf, 1, (size_t) sz_buf, f_out);
	DUK_FFLUSH(f_out);
#endif

#if defined(DUK_USE_DEBUGGER_SUPPORT) && defined(DUK_USE_DEBUGGER_FWD_PRINTALERT)
	if (DUK_HEAP_IS_DEBUGGER_ATTACHED(thr->heap)) {
		duk_debug_write_notify(thr, magic ? DUK_DBG_CMD_ALERT : DUK_DBG_CMD_PRINT);
		duk_debug_write_string(thr, (const char *) buf, sz_buf);
		duk_debug_write_eom(thr);
	}
#endif
	return 0;
}
#elif defined(DUK_USE_BROWSER_LIKE)  /* print provider */
DUK_INTERNAL duk_ret_t duk_bi_global_object_print_helper(duk_context *ctx) {
	DUK_UNREF(ctx);
	return 0;
}
#else  /* print provider */
DUK_INTERNAL duk_ret_t duk_bi_global_object_print_helper(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}
#endif  /* print provider */

/*
 *  CommonJS require() and modules support
 */

#if defined(DUK_USE_COMMONJS_MODULES)
DUK_LOCAL void duk__bi_global_resolve_module_id(duk_context *ctx, const char *req_id, const char *mod_id) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_uint8_t buf[DUK_BI_COMMONJS_MODULE_ID_LIMIT];
	duk_uint8_t *p;
	duk_uint8_t *q;
	duk_uint8_t *q_last;  /* last component */
	duk_int_t int_rc;

	DUK_ASSERT(req_id != NULL);
	/* mod_id may be NULL */

	/*
	 *  A few notes on the algorithm:
	 *
	 *    - Terms are not allowed to begin with a period unless the term
	 *      is either '.' or '..'.  This simplifies implementation (and
	 *      is within CommonJS modules specification).
	 *
	 *    - There are few output bound checks here.  This is on purpose:
	 *      the resolution input is length checked and the output is never
	 *      longer than the input.  The resolved output is written directly
	 *      over the input because it's never longer than the input at any
	 *      point in the algorithm.
	 *
	 *    - Non-ASCII characters are processed as individual bytes and
	 *      need no special treatment.  However, U+0000 terminates the
	 *      algorithm; this is not an issue because U+0000 is not a
	 *      desirable term character anyway.
	 */

	/*
	 *  Set up the resolution input which is the requested ID directly
	 *  (if absolute or no current module path) or with current module
	 *  ID prepended (if relative and current module path exists).
	 *
	 *  Suppose current module is 'foo/bar' and relative path is './quux'.
	 *  The 'bar' component must be replaced so the initial input here is
	 *  'foo/bar/.././quux'.
	 */

	if (mod_id != NULL && req_id[0] == '.') {
		int_rc = DUK_SNPRINTF((char *) buf, sizeof(buf), "%s/../%s", mod_id, req_id);
	} else {
		int_rc = DUK_SNPRINTF((char *) buf, sizeof(buf), "%s", req_id);
	}
	if (int_rc >= (duk_int_t) sizeof(buf) || int_rc < 0) {
		/* Potentially truncated, NUL not guaranteed in any case.
		 * The (int_rc < 0) case should not occur in practice.
		 */
		DUK_DD(DUK_DDPRINT("resolve error: temporary working module ID doesn't fit into resolve buffer"));
		goto resolve_error;
	}
	DUK_ASSERT(DUK_STRLEN((const char *) buf) < sizeof(buf));  /* at most sizeof(buf) - 1 */

	DUK_DDD(DUK_DDDPRINT("input module id: '%s'", (const char *) buf));

	/*
	 *  Resolution loop.  At the top of the loop we're expecting a valid
	 *  term: '.', '..', or a non-empty identifier not starting with a period.
	 */

	p = buf;
	q = buf;
	for (;;) {
		duk_uint_fast8_t c;

		/* Here 'p' always points to the start of a term.
		 *
		 * We can also unconditionally reset q_last here: if this is
		 * the last (non-empty) term q_last will have the right value
		 * on loop exit.
		 */

		DUK_ASSERT(p >= q);  /* output is never longer than input during resolution */

		DUK_DDD(DUK_DDDPRINT("resolve loop top: p -> '%s', q=%p, buf=%p",
		                     (const char *) p, (void *) q, (void *) buf));

		q_last = q;

		c = *p++;
		if (DUK_UNLIKELY(c == 0)) {
			DUK_DD(DUK_DDPRINT("resolve error: requested ID must end with a non-empty term"));
			goto resolve_error;
		} else if (DUK_UNLIKELY(c == '.')) {
			c = *p++;
			if (c == '/') {
				/* Term was '.' and is eaten entirely (including dup slashes). */
				goto eat_dup_slashes;
			}
			if (c == '.' && *p == '/') {
				/* Term was '..', backtrack resolved name by one component.
				 *  q[-1] = previous slash (or beyond start of buffer)
				 *  q[-2] = last char of previous component (or beyond start of buffer)
				 */
				p++;  /* eat (first) input slash */
				DUK_ASSERT(q >= buf);
				if (q == buf) {
					DUK_DD(DUK_DDPRINT("resolve error: term was '..' but nothing to backtrack"));
					goto resolve_error;
				}
				DUK_ASSERT(*(q - 1) == '/');
				q--;  /* backtrack to last output slash (dups already eliminated) */
				for (;;) {
					/* Backtrack to previous slash or start of buffer. */
					DUK_ASSERT(q >= buf);
					if (q == buf) {
						break;
					}
					if (*(q - 1) == '/') {
						break;
					}
					q--;
				}
				goto eat_dup_slashes;
			}
			DUK_DD(DUK_DDPRINT("resolve error: term begins with '.' but is not '.' or '..' (not allowed now)"));
			goto resolve_error;
		} else if (DUK_UNLIKELY(c == '/')) {
			/* e.g. require('/foo'), empty terms not allowed */
			DUK_DD(DUK_DDPRINT("resolve error: empty term (not allowed now)"));
			goto resolve_error;
		} else {
			for (;;) {
				/* Copy term name until end or '/'. */
				*q++ = c;
				c = *p++;
				if (DUK_UNLIKELY(c == 0)) {
					/* This was the last term, and q_last was
					 * updated to match this term at loop top.
					 */
					goto loop_done;
				} else if (DUK_UNLIKELY(c == '/')) {
					*q++ = '/';
					break;
				} else {
					/* write on next loop */
				}
			}
		}

	 eat_dup_slashes:
		for (;;) {
			/* eat dup slashes */
			c = *p;
			if (DUK_LIKELY(c != '/')) {
				break;
			}
			p++;
		}
	}
 loop_done:
	/* Output #1: resolved absolute name */
	DUK_ASSERT(q >= buf);
	duk_push_lstring(ctx, (const char *) buf, (size_t) (q - buf));

	/* Output #2: last component name */
	DUK_ASSERT(q >= q_last);
	DUK_ASSERT(q_last >= buf);
	duk_push_lstring(ctx, (const char *) q_last, (size_t) (q - q_last));

	DUK_DD(DUK_DDPRINT("after resolving module name: buf=%p, q_last=%p, q=%p",
	                   (void *) buf, (void *) q_last, (void *) q));
	return;

 resolve_error:
	DUK_ERROR_FMT1(thr, DUK_ERR_TYPE_ERROR, "cannot resolve module id: %s", (const char *) req_id);
}
#endif  /* DUK_USE_COMMONJS_MODULES */

#if defined(DUK_USE_COMMONJS_MODULES)
/* Stack indices for better readability */
#define DUK__IDX_REQUESTED_ID   0  /* Module id requested */
#define DUK__IDX_REQUIRE        1  /* Current require() function */
#define DUK__IDX_REQUIRE_ID     2  /* The base ID of the current require() function, resolution base */
#define DUK__IDX_RESOLVED_ID    3  /* Resolved, normalized absolute module ID */
#define DUK__IDX_LASTCOMP       4  /* Last component name in resolved path */
#define DUK__IDX_DUKTAPE        5  /* Duktape object */
#define DUK__IDX_MODLOADED      6  /* Duktape.modLoaded[] module cache */
#define DUK__IDX_UNDEFINED      7  /* 'undefined', artifact of lookup */
#define DUK__IDX_FRESH_REQUIRE  8  /* New require() function for module, updated resolution base */
#define DUK__IDX_EXPORTS        9  /* Default exports table */
#define DUK__IDX_MODULE         10  /* Module object containing module.exports, etc */

DUK_INTERNAL duk_ret_t duk_bi_global_object_require(duk_context *ctx) {
	const char *str_req_id;  /* requested identifier */
	const char *str_mod_id;  /* require.id of current module */
	duk_int_t pcall_rc;

	/* NOTE: we try to minimize code size by avoiding unnecessary pops,
	 * so the stack looks a bit cluttered in this function.  DUK_ASSERT_TOP()
	 * assertions are used to ensure stack configuration is correct at each
	 * step.
	 */

	/*
	 *  Resolve module identifier into canonical absolute form.
	 */

	str_req_id = duk_require_string(ctx, DUK__IDX_REQUESTED_ID);
	duk_push_current_function(ctx);
	duk_get_prop_stridx(ctx, -1, DUK_STRIDX_ID);
	str_mod_id = duk_get_string(ctx, DUK__IDX_REQUIRE_ID);  /* ignore non-strings */
	DUK_DDD(DUK_DDDPRINT("resolve module id: requested=%!T, currentmodule=%!T",
	                     duk_get_tval(ctx, DUK__IDX_REQUESTED_ID),
	                     duk_get_tval(ctx, DUK__IDX_REQUIRE_ID)));
	duk__bi_global_resolve_module_id(ctx, str_req_id, str_mod_id);
	str_req_id = NULL;
	str_mod_id = NULL;
	DUK_DDD(DUK_DDDPRINT("resolved module id: requested=%!T, currentmodule=%!T, result=%!T, lastcomp=%!T",
	                     duk_get_tval(ctx, DUK__IDX_REQUESTED_ID),
	                     duk_get_tval(ctx, DUK__IDX_REQUIRE_ID),
	                     duk_get_tval(ctx, DUK__IDX_RESOLVED_ID),
	                     duk_get_tval(ctx, DUK__IDX_LASTCOMP)));

	/* [ requested_id require require.id resolved_id last_comp ] */
	DUK_ASSERT_TOP(ctx, DUK__IDX_LASTCOMP + 1);

	/*
	 *  Cached module check.
	 *
	 *  If module has been loaded or its loading has already begun without
	 *  finishing, return the same cached value ('exports').  The value is
	 *  registered when module load starts so that circular references can
	 *  be supported to some extent.
	 */

	duk_push_hobject_bidx(ctx, DUK_BIDX_DUKTAPE);
	duk_get_prop_stridx(ctx, DUK__IDX_DUKTAPE, DUK_STRIDX_MOD_LOADED);  /* Duktape.modLoaded */
	(void) duk_require_hobject(ctx, DUK__IDX_MODLOADED);
	DUK_ASSERT_TOP(ctx, DUK__IDX_MODLOADED + 1);

	duk_dup(ctx, DUK__IDX_RESOLVED_ID);
	if (duk_get_prop(ctx, DUK__IDX_MODLOADED)) {
		/* [ requested_id require require.id resolved_id last_comp Duktape Duktape.modLoaded Duktape.modLoaded[id] ] */
		DUK_DD(DUK_DDPRINT("module already loaded: %!T",
		                   duk_get_tval(ctx, DUK__IDX_RESOLVED_ID)));
		duk_get_prop_stridx(ctx, -1, DUK_STRIDX_EXPORTS);  /* return module.exports */
		return 1;
	}
	DUK_ASSERT_TOP(ctx, DUK__IDX_UNDEFINED + 1);

	/* [ requested_id require require.id resolved_id last_comp Duktape Duktape.modLoaded undefined ] */

	/*
	 *  Module not loaded (and loading not started previously).
	 *
	 *  Create a new require() function with 'id' set to resolved ID
	 *  of module being loaded.  Also create 'exports' and 'module'
	 *  tables but don't register exports to the loaded table yet.
	 *  We don't want to do that unless the user module search callbacks
	 *  succeeds in finding the module.
	 */

	DUK_D(DUK_DPRINT("loading module %!T, resolution base %!T, requested ID %!T -> resolved ID %!T, last component %!T",
                         duk_get_tval(ctx, DUK__IDX_RESOLVED_ID),
                         duk_get_tval(ctx, DUK__IDX_REQUIRE_ID),
                         duk_get_tval(ctx, DUK__IDX_REQUESTED_ID),
                         duk_get_tval(ctx, DUK__IDX_RESOLVED_ID),
                         duk_get_tval(ctx, DUK__IDX_LASTCOMP)));

	/* Fresh require: require.id is left configurable (but not writable)
	 * so that is not easy to accidentally tweak it, but it can still be
	 * done with Object.defineProperty().
	 *
	 * XXX: require.id could also be just made non-configurable, as there
	 * is no practical reason to touch it.
	 */
	duk_push_c_function(ctx, duk_bi_global_object_require, 1 /*nargs*/);
	duk_push_hstring_stridx(ctx, DUK_STRIDX_REQUIRE);
	duk_xdef_prop_stridx(ctx, DUK__IDX_FRESH_REQUIRE, DUK_STRIDX_NAME, DUK_PROPDESC_FLAGS_NONE);
	duk_dup(ctx, DUK__IDX_RESOLVED_ID);
	duk_xdef_prop_stridx(ctx, DUK__IDX_FRESH_REQUIRE, DUK_STRIDX_ID, DUK_PROPDESC_FLAGS_C);  /* a fresh require() with require.id = resolved target module id */

	/* Module table:
	 * - module.exports: initial exports table (may be replaced by user)
	 * - module.id is non-writable and non-configurable, as the CommonJS
	 *   spec suggests this if possible
	 * - module.filename: not set, defaults to resolved ID if not explicitly
	 *   set by modSearch() (note capitalization, not .fileName, matches Node.js)
	 * - module.name: not set, defaults to last component of resolved ID if
	 *   not explicitly set by modSearch()
	 */
	duk_push_object(ctx);  /* exports */
	duk_push_object(ctx);  /* module */
	duk_dup(ctx, DUK__IDX_EXPORTS);
	duk_xdef_prop_stridx(ctx, DUK__IDX_MODULE, DUK_STRIDX_EXPORTS, DUK_PROPDESC_FLAGS_WC);  /* module.exports = exports */
	duk_dup(ctx, DUK__IDX_RESOLVED_ID);  /* resolved id: require(id) must return this same module */
	duk_xdef_prop_stridx(ctx, DUK__IDX_MODULE, DUK_STRIDX_ID, DUK_PROPDESC_FLAGS_NONE);  /* module.id = resolved_id */
	duk_compact(ctx, DUK__IDX_MODULE);  /* module table remains registered to modLoaded, minimize its size */
	DUK_ASSERT_TOP(ctx, DUK__IDX_MODULE + 1);

	DUK_DD(DUK_DDPRINT("module table created: %!T", duk_get_tval(ctx, DUK__IDX_MODULE)));

	/* [ requested_id require require.id resolved_id last_comp Duktape Duktape.modLoaded undefined fresh_require exports module ] */

	/* Register the module table early to modLoaded[] so that we can
	 * support circular references even in modSearch().  If an error
	 * is thrown, we'll delete the reference.
	 */
	duk_dup(ctx, DUK__IDX_RESOLVED_ID);
	duk_dup(ctx, DUK__IDX_MODULE);
	duk_put_prop(ctx, DUK__IDX_MODLOADED);  /* Duktape.modLoaded[resolved_id] = module */

	/*
	 *  Call user provided module search function and build the wrapped
	 *  module source code (if necessary).  The module search function
	 *  can be used to implement pure Ecmacsript, pure C, and mixed
	 *  Ecmascript/C modules.
	 *
	 *  The module search function can operate on the exports table directly
	 *  (e.g. DLL code can register values to it).  It can also return a
	 *  string which is interpreted as module source code (if a non-string
	 *  is returned the module is assumed to be a pure C one).  If a module
	 *  cannot be found, an error must be thrown by the user callback.
	 *
	 *  Because Duktape.modLoaded[] already contains the module being
	 *  loaded, circular references for C modules should also work
	 *  (although expected to be quite rare).
	 */

	duk_push_string(ctx, "(function(require,exports,module){");

	/* Duktape.modSearch(resolved_id, fresh_require, exports, module). */
	duk_get_prop_stridx(ctx, DUK__IDX_DUKTAPE, DUK_STRIDX_MOD_SEARCH);  /* Duktape.modSearch */
	duk_dup(ctx, DUK__IDX_RESOLVED_ID);
	duk_dup(ctx, DUK__IDX_FRESH_REQUIRE);
	duk_dup(ctx, DUK__IDX_EXPORTS);
	duk_dup(ctx, DUK__IDX_MODULE);  /* [ ... Duktape.modSearch resolved_id last_comp fresh_require exports module ] */
	pcall_rc = duk_pcall(ctx, 4 /*nargs*/);  /* -> [ ... source ] */
	DUK_ASSERT_TOP(ctx, DUK__IDX_MODULE + 3);

	if (pcall_rc != DUK_EXEC_SUCCESS) {
		/* Delete entry in Duktape.modLoaded[] and rethrow. */
		goto delete_rethrow;
	}

	/* If user callback did not return source code, module loading
	 * is finished (user callback initialized exports table directly).
	 */
	if (!duk_is_string(ctx, -1)) {
		/* User callback did not return source code, so module loading
		 * is finished: just update modLoaded with final module.exports
		 * and we're done.
		 */
		goto return_exports;
	}

	/* Finish the wrapped module source.  Force module.filename as the
	 * function .fileName so it gets set for functions defined within a
	 * module.  This also ensures loggers created within the module get
	 * the module ID (or overridden filename) as their default logger name.
	 * (Note capitalization: .filename matches Node.js while .fileName is
	 * used elsewhere in Duktape.)
	 */
	duk_push_string(ctx, "})");
	duk_concat(ctx, 3);
	if (!duk_get_prop_stridx(ctx, DUK__IDX_MODULE, DUK_STRIDX_FILENAME)) {
		/* module.filename for .fileName, default to resolved ID if
		 * not present.
		 */
		duk_pop(ctx);
		duk_dup(ctx, DUK__IDX_RESOLVED_ID);
	}
	duk_eval_raw(ctx, NULL, 0, DUK_COMPILE_EVAL);

	/* Module has now evaluated to a wrapped module function.  Force its
	 * .name to match module.name (defaults to last component of resolved
	 * ID) so that it is shown in stack traces too.  Note that we must not
	 * introduce an actual name binding into the function scope (which is
	 * usually the case with a named function) because it would affect the
	 * scope seen by the module and shadow accesses to globals of the same name.
	 * This is now done by compiling the function as anonymous and then forcing
	 * its .name without setting a "has name binding" flag.
	 */

	duk_push_hstring_stridx(ctx, DUK_STRIDX_NAME);
	if (!duk_get_prop_stridx(ctx, DUK__IDX_MODULE, DUK_STRIDX_NAME)) {
		/* module.name for .name, default to last component if
		 * not present.
		 */
		duk_pop(ctx);
		duk_dup(ctx, DUK__IDX_LASTCOMP);
	}
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_FORCE);

	/*
	 *  Call the wrapped module function.
	 *
	 *  Use a protected call so that we can update Duktape.modLoaded[resolved_id]
	 *  even if the module throws an error.
	 */

	/* [ requested_id require require.id resolved_id last_comp Duktape Duktape.modLoaded undefined fresh_require exports module mod_func ] */
	DUK_ASSERT_TOP(ctx, DUK__IDX_MODULE + 2);

	duk_dup(ctx, DUK__IDX_EXPORTS);  /* exports (this binding) */
	duk_dup(ctx, DUK__IDX_FRESH_REQUIRE);  /* fresh require (argument) */
	duk_get_prop_stridx(ctx, DUK__IDX_MODULE, DUK_STRIDX_EXPORTS);  /* relookup exports from module.exports in case it was changed by modSearch */
	duk_dup(ctx, DUK__IDX_MODULE);  /* module (argument) */
	DUK_ASSERT_TOP(ctx, DUK__IDX_MODULE + 6);

	/* [ requested_id require require.id resolved_id last_comp Duktape Duktape.modLoaded undefined fresh_require exports module mod_func exports fresh_require exports module ] */

	pcall_rc = duk_pcall_method(ctx, 3 /*nargs*/);
	if (pcall_rc != DUK_EXEC_SUCCESS) {
		/* Module loading failed.  Node.js will forget the module
		 * registration so that another require() will try to load
		 * the module again.  Mimic that behavior.
		 */
		goto delete_rethrow;
	}

	/* [ requested_id require require.id resolved_id last_comp Duktape Duktape.modLoaded undefined fresh_require exports module result(ignored) ] */
	DUK_ASSERT_TOP(ctx, DUK__IDX_MODULE + 2);

	/* fall through */

 return_exports:
	duk_get_prop_stridx(ctx, DUK__IDX_MODULE, DUK_STRIDX_EXPORTS);
	duk_compact(ctx, -1);  /* compact the exports table */
	return 1;  /* return module.exports */

 delete_rethrow:
	duk_dup(ctx, DUK__IDX_RESOLVED_ID);
	duk_del_prop(ctx, DUK__IDX_MODLOADED);  /* delete Duktape.modLoaded[resolved_id] */
	duk_throw(ctx);  /* rethrow original error */
	return 0;  /* not reachable */
}

#undef DUK__IDX_REQUESTED_ID
#undef DUK__IDX_REQUIRE
#undef DUK__IDX_REQUIRE_ID
#undef DUK__IDX_RESOLVED_ID
#undef DUK__IDX_LASTCOMP
#undef DUK__IDX_DUKTAPE
#undef DUK__IDX_MODLOADED
#undef DUK__IDX_UNDEFINED
#undef DUK__IDX_FRESH_REQUIRE
#undef DUK__IDX_EXPORTS
#undef DUK__IDX_MODULE
#else
DUK_INTERNAL duk_ret_t duk_bi_global_object_require(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}
#endif  /* DUK_USE_COMMONJS_MODULES */
