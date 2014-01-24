/*
 *  Global object built-ins
 */

#include "duk_internal.h"

/*
 *  Encoding/decoding helpers
 */

/* Macros for creating and checking bitmasks for character encoding.
 * Bit number is a bit counterintuitive, but minimizes code size.
 */
#define MKBITS(a,b,c,d,e,f,g,h)  ((unsigned char) ( \
	((a) << 0) | ((b) << 1) | ((c) << 2) | ((d) << 3) | \
	((e) << 4) | ((f) << 5) | ((g) << 6) | ((h) << 7) \
	))
#define CHECK_BITMASK(table,cp)  ((table)[(cp) >> 3] & (1 << ((cp) & 0x07)))

/* E5.1 Section 15.1.3.3: uriReserved + uriUnescaped + '#' */
static unsigned char encode_uri_unescaped_table[16] = {
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x00-0x0f */
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x10-0x1f */
	MKBITS(0, 1, 0, 1, 1, 0, 1, 1), MKBITS(1, 1, 1, 1, 1, 1, 1, 1),  /* 0x20-0x2f */
	MKBITS(1, 1, 1, 1, 1, 1, 1, 1), MKBITS(1, 1, 1, 1, 0, 1, 0, 1),  /* 0x30-0x3f */
	MKBITS(1, 1, 1, 1, 1, 1, 1, 1), MKBITS(1, 1, 1, 1, 1, 1, 1, 1),  /* 0x40-0x4f */
	MKBITS(1, 1, 1, 1, 1, 1, 1, 1), MKBITS(1, 1, 1, 0, 0, 0, 0, 1),  /* 0x50-0x5f */
	MKBITS(0, 1, 1, 1, 1, 1, 1, 1), MKBITS(1, 1, 1, 1, 1, 1, 1, 1),  /* 0x60-0x6f */
	MKBITS(1, 1, 1, 1, 1, 1, 1, 1), MKBITS(1, 1, 1, 0, 0, 0, 1, 0),  /* 0x70-0x7f */
};

/* E5.1 Section 15.1.3.4: uriUnescaped */
static unsigned char encode_uri_component_unescaped_table[16] = {
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x00-0x0f */
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x10-0x1f */
	MKBITS(0, 1, 0, 0, 0, 0, 0, 1), MKBITS(1, 1, 1, 0, 0, 1, 1, 0),  /* 0x20-0x2f */
	MKBITS(1, 1, 1, 1, 1, 1, 1, 1), MKBITS(1, 1, 0, 0, 0, 0, 0, 0),  /* 0x30-0x3f */
	MKBITS(0, 1, 1, 1, 1, 1, 1, 1), MKBITS(1, 1, 1, 1, 1, 1, 1, 1),  /* 0x40-0x4f */
	MKBITS(1, 1, 1, 1, 1, 1, 1, 1), MKBITS(1, 1, 1, 0, 0, 0, 0, 1),  /* 0x50-0x5f */
	MKBITS(0, 1, 1, 1, 1, 1, 1, 1), MKBITS(1, 1, 1, 1, 1, 1, 1, 1),  /* 0x60-0x6f */
	MKBITS(1, 1, 1, 1, 1, 1, 1, 1), MKBITS(1, 1, 1, 0, 0, 0, 1, 0),  /* 0x70-0x7f */
};

/* E5.1 Section 15.1.3.1: uriReserved + '#' */
static unsigned char decode_uri_reserved_table[16] = {
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x00-0x0f */
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x10-0x1f */
	MKBITS(0, 0, 0, 1, 1, 0, 1, 0), MKBITS(0, 0, 0, 1, 1, 0, 0, 1),  /* 0x20-0x2f */
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 1, 1, 0, 1, 0, 1),  /* 0x30-0x3f */
	MKBITS(1, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x40-0x4f */
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x50-0x5f */
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x60-0x6f */
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x70-0x7f */
};

/* E5.1 Section 15.1.3.2: empty */
static unsigned char decode_uri_component_reserved_table[16] = {
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x00-0x0f */
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x10-0x1f */
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x20-0x2f */
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x30-0x3f */
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x40-0x4f */
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x50-0x5f */
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x60-0x6f */
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x70-0x7f */
};

#ifdef DUK_USE_SECTION_B
/* E5.1 Section B.2.2, step 7. */
static unsigned char escape_unescaped_table[16] = {
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x00-0x0f */
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 0, 0, 0, 0, 0, 0),  /* 0x10-0x1f */
	MKBITS(0, 0, 0, 0, 0, 0, 0, 0), MKBITS(0, 0, 1, 1, 0, 1, 1, 1),  /* 0x20-0x2f */
	MKBITS(1, 1, 1, 1, 1, 1, 1, 1), MKBITS(1, 1, 0, 0, 0, 0, 0, 0),  /* 0x30-0x3f */
	MKBITS(1, 1, 1, 1, 1, 1, 1, 1), MKBITS(1, 1, 1, 1, 1, 1, 1, 1),  /* 0x40-0x4f */
	MKBITS(1, 1, 1, 1, 1, 1, 1, 1), MKBITS(1, 1, 1, 0, 0, 0, 0, 1),  /* 0x50-0x5f */
	MKBITS(0, 1, 1, 1, 1, 1, 1, 1), MKBITS(1, 1, 1, 1, 1, 1, 1, 1),  /* 0x60-0x6f */
	MKBITS(1, 1, 1, 1, 1, 1, 1, 1), MKBITS(1, 1, 1, 0, 0, 0, 0, 0)   /* 0x70-0x7f */
};
#endif  /* DUK_USE_SECTION_B */

typedef struct {
	duk_hthread *thr;
	duk_hstring *h_str;
	duk_hbuffer_dynamic *h_buf;
	duk_uint8_t *p;
	duk_uint8_t *p_start;
	duk_uint8_t *p_end;
} duk_transform_context;

typedef void (*duk_transform_callback)(duk_transform_context *tfm_ctx, void *udata, duk_codepoint_t cp);

/* FIXME: refactor and share with other code */
static duk_small_int_t decode_hex_escape(duk_uint8_t *p, duk_small_int_t n) {
	duk_small_int_t ch;
	duk_small_int_t t = 0;

	while (n > 0) {
		t = t * 16;
		ch = (int) (*p++);
		if (ch >= (int) '0' && ch <= (int) '9') {
			t += ch - ((int) '0');
		} else if (ch >= (int) 'a' && ch <= (int) 'f') {
			t += ch - ((int) 'a') + 0x0a;
		} else if (ch >= (int) 'A' && ch <= (int) 'F') {
			t += ch - ((int) 'A') + 0x0a;
		} else {
			return -1;
		}
		n--;
	}
	return t;
}

static int transform_helper(duk_context *ctx, duk_transform_callback callback, void *udata) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_transform_context tfm_ctx_alloc;
	duk_transform_context *tfm_ctx = &tfm_ctx_alloc;
	duk_codepoint_t cp;

	tfm_ctx->thr = thr;

	tfm_ctx->h_str = duk_to_hstring(ctx, 0);
	DUK_ASSERT(tfm_ctx->h_str != NULL);

	(void) duk_push_dynamic_buffer(ctx, 0);
	tfm_ctx->h_buf = (duk_hbuffer_dynamic *) duk_get_hbuffer(ctx, -1);
	DUK_ASSERT(tfm_ctx->h_buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(tfm_ctx->h_buf));

	tfm_ctx->p_start = DUK_HSTRING_GET_DATA(tfm_ctx->h_str);
	tfm_ctx->p_end = tfm_ctx->p_start + DUK_HSTRING_GET_BYTELEN(tfm_ctx->h_str);
	tfm_ctx->p = tfm_ctx->p_start;

	while (tfm_ctx->p < tfm_ctx->p_end) {
		cp = (duk_codepoint_t) duk_unicode_decode_xutf8_checked(thr, &tfm_ctx->p, tfm_ctx->p_start, tfm_ctx->p_end);
		callback(tfm_ctx, udata, cp);
	}

	duk_to_string(ctx, -1);
	return 1;
}

static void duk_transform_callback_encode_uri(duk_transform_context *tfm_ctx, void *udata, duk_codepoint_t cp) {
	duk_uint8_t xutf8_buf[DUK_UNICODE_MAX_XUTF8_LENGTH];
	duk_uint8_t buf[3];
	duk_small_int_t len;
	duk_codepoint_t cp1, cp2;
	duk_small_int_t i, t;
	duk_uint8_t *unescaped_table = (duk_uint8_t *) udata;

	if (cp < 0) {
		goto uri_error;
	} else if ((cp < 0x80L) && CHECK_BITMASK(unescaped_table, cp)) {
		duk_hbuffer_append_byte(tfm_ctx->thr, tfm_ctx->h_buf, (duk_uint8_t) cp);
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
		 * They'll decode back into surrogate pairs.
		 */
		;
	}

	len = duk_unicode_encode_xutf8((duk_ucodepoint_t) cp, xutf8_buf);
	buf[0] = (duk_uint8_t) '%';
	for (i = 0; i < len; i++) {
		t = (int) xutf8_buf[i];
		buf[1] = (duk_uint8_t) duk_uc_nybbles[t >> 4];
		buf[2] = (duk_uint8_t) duk_uc_nybbles[t & 0x0f];
		duk_hbuffer_append_bytes(tfm_ctx->thr, tfm_ctx->h_buf, buf, 3);
	}
	return;

 uri_error:
	DUK_ERROR(tfm_ctx->thr, DUK_ERR_URI_ERROR, "invalid input");
}

static void duk_transform_callback_decode_uri(duk_transform_context *tfm_ctx, void *udata, duk_codepoint_t cp) {
	duk_uint8_t *reserved_table = (duk_uint8_t *) udata;
	duk_small_uint_t utf8_blen;
	duk_codepoint_t min_cp;
	duk_small_int_t t;  /* must be signed */
	duk_small_uint_t i;

	if (cp == (duk_codepoint_t) '%') {
		duk_uint8_t *p = tfm_ctx->p;
		duk_size_t left = (duk_size_t) (tfm_ctx->p_end - p);  /* bytes left */

		DUK_DDDPRINT("percent encoding, left=%d", (int) left);

		if (left < 2) {
			goto uri_error;
		}

		t = decode_hex_escape(p, 2);
		DUK_DDDPRINT("first byte: %d", t);
		if (t < 0) {
			goto uri_error;
		}

		if (t < 128) {
			if (CHECK_BITMASK(reserved_table, t)) {
				/* decode '%xx' to '%xx' if decoded char in reserved set */
				DUK_ASSERT(tfm_ctx->p - 1 >= tfm_ctx->p_start);
				duk_hbuffer_append_bytes(tfm_ctx->thr, tfm_ctx->h_buf, (duk_uint8_t *) (p - 1), 3);
			} else {
				duk_hbuffer_append_byte(tfm_ctx->thr, tfm_ctx->h_buf, (duk_uint8_t) t);
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
			t = decode_hex_escape(p, 2);
			DUK_DDDPRINT("i=%d utf8_blen=%d cp=%d t=0x%02x", i, utf8_blen, cp,t);
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

		DUK_DDDPRINT("final cp=%d, min_cp=%d", cp, min_cp);

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
			duk_hbuffer_append_xutf8(tfm_ctx->thr, tfm_ctx->h_buf, (duk_ucodepoint_t) ((cp >> 10) + 0xd800L));
			duk_hbuffer_append_xutf8(tfm_ctx->thr, tfm_ctx->h_buf, (duk_ucodepoint_t) ((cp & 0x03ffUL) + 0xdc00L));
		} else {
			duk_hbuffer_append_xutf8(tfm_ctx->thr, tfm_ctx->h_buf, (duk_ucodepoint_t) cp);
		}
	} else {
		duk_hbuffer_append_xutf8(tfm_ctx->thr, tfm_ctx->h_buf, (duk_ucodepoint_t) cp);
	}
	return;

 uri_error:
	DUK_ERROR(tfm_ctx->thr, DUK_ERR_URI_ERROR, "invalid input");
}

#ifdef DUK_USE_SECTION_B
static void duk_transform_callback_escape(duk_transform_context *tfm_ctx, void *udata, duk_codepoint_t cp) {
	duk_uint8_t buf[6];
	duk_small_int_t len;

	DUK_UNREF(udata);

	if (cp < 0) {
		goto esc_error;
	} else if ((cp < 0x80L) && CHECK_BITMASK(escape_unescaped_table, cp)) {
		buf[0] = (duk_uint8_t) cp;
		len = 1;
	} else if (cp < 0x100L) {
		buf[0] = (duk_uint8_t) '%';
		buf[1] = (duk_uint8_t) duk_uc_nybbles[cp >> 4];
		buf[2] = (duk_uint8_t) duk_uc_nybbles[cp & 0x0f];
		len = 3;
	} else if (cp < 0x10000L) {
		buf[0] = (duk_uint8_t) '%';
		buf[1] = (duk_uint8_t) 'u';
		buf[2] = (duk_uint8_t) duk_uc_nybbles[cp >> 12];
		buf[3] = (duk_uint8_t) duk_uc_nybbles[(cp >> 8) & 0x0f];
		buf[4] = (duk_uint8_t) duk_uc_nybbles[(cp >> 4) & 0x0f];
		buf[5] = (duk_uint8_t) duk_uc_nybbles[cp & 0x0f];
		len = 6;
	} else {
		/* Characters outside BMP cannot be escape()'d.  We could
		 * encode them as surrogate pairs (for codepoints inside
		 * valid UTF-8 range, but not extended UTF-8).  Because
		 * escape() and unescape() are legacy functions, we don't.
		 */
		goto esc_error;
	}

	duk_hbuffer_append_bytes(tfm_ctx->thr, tfm_ctx->h_buf, buf, len);
	return;

 esc_error:
	DUK_ERROR(tfm_ctx->thr, DUK_ERR_TYPE_ERROR, "invalid input");
}

static void duk_transform_callback_unescape(duk_transform_context *tfm_ctx, void *udata, duk_codepoint_t cp) {
	duk_small_int_t t;

	DUK_UNREF(udata);

	if (cp == (duk_codepoint_t) '%') {
		duk_uint8_t *p = tfm_ctx->p;
		duk_size_t left = (duk_size_t) (tfm_ctx->p_end - p);  /* bytes left */

		if (left >= 5 && p[0] == 'u' &&
		    ((t = decode_hex_escape(p + 1, 4)) >= 0)) {
			cp = (duk_codepoint_t) t;
			tfm_ctx->p += 5;
		} else if (left >= 2 &&
		    ((t = decode_hex_escape(p, 2)) >= 0)) {
			cp = (duk_codepoint_t) t;
			tfm_ctx->p += 2;
		}
	}

	duk_hbuffer_append_xutf8(tfm_ctx->thr, tfm_ctx->h_buf, cp);
}
#endif  /* DUK_USE_SECTION_B */

/*
 *  Eval
 */

int duk_bi_global_object_eval(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h;
	duk_activation *act_caller;
	duk_activation *act_eval;
	duk_activation *act;
	duk_hcompiledfunction *func;
	duk_hobject *outer_lex_env;
	duk_hobject *outer_var_env;
	int this_to_global = 1;
	int comp_flags;

	DUK_ASSERT_TOP(ctx, 1);

	if (thr->callstack_top < 2) {
		/* callstack_top - 1 --> this function
		 * callstack_top - 2 --> caller
		 *
		 * If called directly from C, callstack_top might be 1.
		 * We don't support that now.
		 */
		return DUK_RET_TYPE_ERROR;
	}
	DUK_ASSERT(thr->callstack_top >= 2);  /* caller and this function */

	h = duk_get_hstring(ctx, 0);
	if (!h) {
		return 1;  /* return arg as-is */
	}

	/* FIXME: uses internal API */

	comp_flags = DUK_JS_COMPILE_FLAG_EVAL;
	act_caller = thr->callstack + thr->callstack_top - 2;  /* caller */
	act_eval = thr->callstack + thr->callstack_top - 1;    /* this function */
	if ((act_caller->flags & DUK_ACT_FLAG_STRICT) &&
	    (act_eval->flags & DUK_ACT_FLAG_DIRECT_EVAL)) {
		/* Only direct eval inherits strictness from calling code
		 * (E5.1 Section 10.1.1).
		 */
		comp_flags |= DUK_JS_COMPILE_FLAG_STRICT;
	}
	act_caller = NULL;  /* avoid dereference after potential callstack realloc */
	act_eval = NULL;

	duk_push_hstring_stridx(ctx, DUK_STRIDX_INPUT);  /* XXX: copy from caller? */
	duk_js_compile(thr, comp_flags);
	func = (duk_hcompiledfunction *) duk_get_hobject(ctx, -1);
	DUK_ASSERT(func != NULL);
	DUK_ASSERT(DUK_HOBJECT_IS_COMPILEDFUNCTION((duk_hobject *) func));

	/* E5 Section 10.4.2 */
	DUK_ASSERT(thr->callstack_top >= 2);
	act = thr->callstack + thr->callstack_top - 1;  /* this function */
	if (act->flags & DUK_ACT_FLAG_DIRECT_EVAL) {	
		act = thr->callstack + thr->callstack_top - 2;  /* caller */
		if (act->lex_env == NULL) {
			DUK_ASSERT(act->var_env == NULL);
			DUK_DDDPRINT("delayed environment initialization");

			/* this may have side effects, so re-lookup act */
			duk_js_init_activation_environment_records_delayed(thr, act);
			act = thr->callstack + thr->callstack_top - 2;
		}
		DUK_ASSERT(act->lex_env != NULL);
		DUK_ASSERT(act->var_env != NULL);

		this_to_global = 0;

		if (DUK_HOBJECT_HAS_STRICT((duk_hobject *) func)) {
			duk_hobject *new_env;

			DUK_DDDPRINT("direct eval call to a strict function -> "
			             "var_env and lex_env to a fresh env, "
			             "this_binding to caller's this_binding");

			(void) duk_push_object_helper(ctx,
			                              DUK_HOBJECT_FLAG_EXTENSIBLE |
			                              DUK_HOBJECT_CLASS_AS_FLAGS(DUK_HOBJECT_CLASS_DECENV),
			                              -1);  /* no prototype, updated below */
			new_env = duk_require_hobject(ctx, -1);
			DUK_ASSERT(new_env != NULL);
			DUK_DDDPRINT("new_env allocated: %!iO", new_env);

			act = thr->callstack + thr->callstack_top - 2;  /* caller */
			DUK_HOBJECT_SET_PROTOTYPE(thr, new_env, act->lex_env);  /* updates refcounts */
			act = NULL;  /* invalidated */

			outer_lex_env = new_env;
			outer_var_env = new_env;

			duk_insert(ctx, 0);  /* stash to bottom of value stack to keep new_env reachable */

			/* compiler's responsibility */
			DUK_ASSERT(DUK_HOBJECT_HAS_NEWENV((duk_hobject *) func));
		} else {
			DUK_DDDPRINT("direct eval call to a non-strict function -> "
			             "var_env and lex_env to caller's envs, "
			             "this_binding to caller's this_binding");

			outer_lex_env = act->lex_env;
			outer_var_env = act->var_env;

			/* compiler's responsibility */
			DUK_ASSERT(!DUK_HOBJECT_HAS_NEWENV((duk_hobject *) func));
		}
	} else {
		DUK_DDDPRINT("indirect eval call -> var_env and lex_env to "
		             "global object, this_binding to global object");

		this_to_global = 1;
		outer_lex_env = thr->builtins[DUK_BIDX_GLOBAL_ENV];
		outer_var_env = thr->builtins[DUK_BIDX_GLOBAL_ENV];
	}
	act = NULL;

	duk_js_push_closure(thr, func, outer_var_env, outer_lex_env);

	if (this_to_global) {
		DUK_ASSERT(thr->builtins[DUK_BIDX_GLOBAL] != NULL);
		duk_push_hobject(ctx, thr->builtins[DUK_BIDX_GLOBAL]);
	} else {
		duk_tval *tv;
		DUK_ASSERT(thr->callstack_top >= 2);
		act = thr->callstack + thr->callstack_top - 2;  /* caller */
		tv = thr->valstack + act->idx_bottom - 1;  /* this is just beneath bottom */
		DUK_ASSERT(tv >= thr->valstack);
		duk_push_tval(ctx, tv);
	}

	DUK_DDDPRINT("eval -> lex_env=%!iO, var_env=%!iO, this_binding=%!T",
	             outer_lex_env, outer_var_env, duk_get_tval(ctx, -1));

	duk_call_method(ctx, 0);

	return 1;
}

/*
 *  Parsing of ints and floats
 */

int duk_bi_global_object_parse_int(duk_context *ctx) {
	int strip_prefix;
	duk_int32_t radix;
	int s2n_flags;

	DUK_ASSERT_TOP(ctx, 2);
	duk_to_string(ctx, 0);

	strip_prefix = 1;
	radix = duk_to_int32(ctx, 1);
	if (radix != 0) {
		if (radix < 2 || radix > 36) {
			goto ret_nan;
		}
		/* FIXME: how should octal behave here? */
		if (radix != 16) {
			strip_prefix = 0;
		}
	} else {
		radix = 10;
	}

	s2n_flags = DUK_S2N_FLAG_TRIM_WHITE |
	            DUK_S2N_FLAG_ALLOW_GARBAGE |
	            DUK_S2N_FLAG_ALLOW_PLUS |
	            DUK_S2N_FLAG_ALLOW_MINUS |
	            DUK_S2N_FLAG_ALLOW_LEADING_ZERO |
#ifdef DUK_USE_OCTAL_SUPPORT
	            (strip_prefix ? DUK_S2N_FLAG_ALLOW_AUTO_OCT_INT : 0) |
#endif
	            (strip_prefix ? DUK_S2N_FLAG_ALLOW_AUTO_HEX_INT : 0);

	duk_dup(ctx, 0);
	duk_numconv_parse(ctx, radix, s2n_flags);
	return 1;

 ret_nan:
	duk_push_nan(ctx);
	return 1;
}

int duk_bi_global_object_parse_float(duk_context *ctx) {
	int s2n_flags;

	DUK_ASSERT_TOP(ctx, 1);
	duk_to_string(ctx, 0);

	/* FIXME: flags */
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

	duk_numconv_parse(ctx, 10 /*radix*/, s2n_flags);
	return 1;
}

/*
 *  Number checkers
 */
int duk_bi_global_object_is_nan(duk_context *ctx) {
	double d = duk_to_number(ctx, 0);
	duk_push_boolean(ctx, DUK_ISNAN(d));
	return 1;
}

int duk_bi_global_object_is_finite(duk_context *ctx) {
	double d = duk_to_number(ctx, 0);
	duk_push_boolean(ctx, DUK_ISFINITE(d));
	return 1;
}

/*
 *  URI handling
 */

int duk_bi_global_object_decode_uri(duk_context *ctx) {
	return transform_helper(ctx, duk_transform_callback_decode_uri, (void *) decode_uri_reserved_table);
}

int duk_bi_global_object_decode_uri_component(duk_context *ctx) {
	return transform_helper(ctx, duk_transform_callback_decode_uri, (void *) decode_uri_component_reserved_table);
}

int duk_bi_global_object_encode_uri(duk_context *ctx) {
	return transform_helper(ctx, duk_transform_callback_encode_uri, (void *) encode_uri_unescaped_table);
}

int duk_bi_global_object_encode_uri_component(duk_context *ctx) {
	return transform_helper(ctx, duk_transform_callback_encode_uri, (void *) encode_uri_component_unescaped_table);
}

#ifdef DUK_USE_SECTION_B
int duk_bi_global_object_escape(duk_context *ctx) {
	return transform_helper(ctx, duk_transform_callback_escape, (void *) NULL);
}

int duk_bi_global_object_unescape(duk_context *ctx) {
	return transform_helper(ctx, duk_transform_callback_unescape, (void *) NULL);
}
#else  /* DUK_USE_SECTION_B */
int duk_bi_global_object_escape(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}

int duk_bi_global_object_unescape(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}
#endif  /* DUK_USE_SECTION_B */

#ifdef DUK_USE_BROWSER_LIKE
#ifdef DUK_USE_FILE_IO
static int print_alert_helper(duk_context *ctx, FILE *f_out) {
	int nargs;
	int i;
	const char *str;
	size_t len;
	char nl = '\n';

	/* If argument count is 1 and first argument is a buffer, write the buffer
	 * as raw data into the file without a newline; this allows exact control
	 * over stdout/stderr without an additional entrypoint (useful for now).
	 */

	nargs = duk_get_top(ctx);
	if (nargs == 1 && duk_is_buffer(ctx, 0)) {
		const char *buf = NULL;
		size_t sz = 0;
		buf = (const char *) duk_get_buffer(ctx, 0, &sz);
		if (buf && sz > 0) {
			fwrite(buf, 1, sz, f_out);
		}
		goto flush;
	}

	/* FIXME: best semantics link?  Now apply ToString to args, join with ' ' */
	/* FIXME: ToString() coerce inplace instead? */

	if (nargs > 0) {
		for (i = 0; i < nargs; i++) {
			if (i != 0) {
				duk_push_hstring_stridx(ctx, DUK_STRIDX_SPACE);
			}
			duk_dup(ctx, i);
			duk_to_string(ctx, -1);
		}

		duk_concat(ctx, 2*nargs - 1);

		str = duk_get_lstring(ctx, -1, &len);
		if (str) {
			fwrite(str, 1, len, f_out);
		}
	}

	fwrite(&nl, 1, 1, f_out);

 flush:
	fflush(f_out);
	return 0;
}

int duk_bi_global_object_print(duk_context *ctx) {
	return print_alert_helper(ctx, stdout);
}

int duk_bi_global_object_alert(duk_context *ctx) {
	return print_alert_helper(ctx, stderr);
}
#else  /* DUK_USE_FILE_IO */
/* Supported but no file I/O -> silently ignore, no error */
int duk_bi_global_object_print(duk_context *ctx) {
	return 0;
}

int duk_bi_global_object_alert(duk_context *ctx) {
	return 0;
}
#endif  /* DUK_USE_FILE_IO */
#else  /* DUK_USE_BROWSER_LIKE */
int duk_bi_global_object_print(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}

int duk_bi_global_object_alert(duk_context *ctx) {
	DUK_UNREF(ctx);
	return DUK_RET_UNSUPPORTED_ERROR;
}
#endif  /* DUK_USE_BROWSER_LIKE */
