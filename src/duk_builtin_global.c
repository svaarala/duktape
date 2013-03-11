/*
 *  Global object built-in.
 */

#include "duk_internal.h"

/*
 *  Encoding/decoding helpers
 */

/* Macros for creating and checking bitmasks for character encoding.
 * Bit number is a bit counterintuitive, but minimizes code size.
 */
#define  MKBITS(a,b,c,d,e,f,g,h)  ((unsigned char) ( \
	((a) << 0) | ((b) << 1) | ((c) << 2) | ((d) << 3) | \
	((e) << 4) | ((f) << 5) | ((g) << 6) | ((h) << 7) \
	))
#define  CHECK_BITMASK(table,cp)  ((table)[(cp) >> 3] & (1 << ((cp) & 0x07)))

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
	duk_hbuffer_growable *h_buf;
	duk_u8 *p;
	duk_u8 *p_start;
	duk_u8 *p_end;
} duk_transform_context;

typedef void (*transform_callback)(duk_transform_context *tfm_ctx, void *udata, duk_u32 cp);

/* FIXME: refactor and share with other code */
static int decode_hex_escape(duk_u8 *p, int n) {
	int ch;
	int t = 0;

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

static int transform_helper(duk_context *ctx, transform_callback callback, void *udata) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_transform_context tfm_ctx_alloc;
	duk_transform_context *tfm_ctx = &tfm_ctx_alloc;
	duk_u32 cp;

	tfm_ctx->thr = thr;

	tfm_ctx->h_str = duk_to_hstring(ctx, 0);
	DUK_ASSERT(tfm_ctx->h_str != NULL);

	(void) duk_push_new_growable_buffer(ctx, 0);
	tfm_ctx->h_buf = (duk_hbuffer_growable *) duk_get_hbuffer(ctx, -1);
	DUK_ASSERT(tfm_ctx->h_buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(tfm_ctx->h_buf));

	tfm_ctx->p_start = DUK_HSTRING_GET_DATA(tfm_ctx->h_str);
	tfm_ctx->p_end = tfm_ctx->p_start + DUK_HSTRING_GET_BYTELEN(tfm_ctx->h_str);
	tfm_ctx->p = tfm_ctx->p_start;

	while (tfm_ctx->p < tfm_ctx->p_end) {
		cp = duk_unicode_xutf8_get_u32_checked(thr, &tfm_ctx->p, tfm_ctx->p_start, tfm_ctx->p_end);
		callback(tfm_ctx, udata, cp);
	}

	duk_to_string(ctx, -1);
	return 1;
}

static void transform_callback_encode_uri(duk_transform_context *tfm_ctx, void *udata, duk_u32 cp) {
	duk_u8 xutf8_buf[DUK_UNICODE_MAX_XUTF8_LENGTH];
	duk_u8 buf[3];
	size_t len;
	duk_u32 cp1, cp2;
	int i, t;
	duk_u8 *unescaped_table = (duk_u8 *) udata;

	if ((cp < 128) && CHECK_BITMASK(unescaped_table, cp)) {
		duk_hbuffer_append_byte(tfm_ctx->thr, tfm_ctx->h_buf, (duk_u8) cp);
		return;
	} else if (cp >= 0xdc00 && cp <= 0xdfff) {
		goto uri_error;
	} else if (cp >= 0xd800 && cp <= 0xdbff) {
		/* Needs lookahead */
		if (duk_unicode_xutf8_get_u32(tfm_ctx->thr, &tfm_ctx->p, tfm_ctx->p_start, tfm_ctx->p_end, &cp2) == 0) {
			goto uri_error;
		}
		if (!(cp2 >= 0xdc00 && cp2 <= 0xdfff)) {
			goto uri_error;
		}
		cp1 = cp;
		cp = ((cp1 - 0xd800) << 10) + (cp2 - 0xdc00) + 0x10000;
	} else if (cp > 0x10ffff) {
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

	len = duk_unicode_encode_xutf8(cp, xutf8_buf);
	buf[0] = (duk_u8) '%';
	for (i = 0; i < len; i++) {
		t = (int) xutf8_buf[i];
		buf[1] = (duk_u8) duk_uc_nybbles[t >> 4];
		buf[2] = (duk_u8) duk_uc_nybbles[t & 0x0f];
		duk_hbuffer_append_bytes(tfm_ctx->thr, tfm_ctx->h_buf, buf, 3);
	}
	return;

 uri_error:
	DUK_ERROR(tfm_ctx->thr, DUK_ERR_URI_ERROR, "invalid input");
}

static void transform_callback_decode_uri(duk_transform_context *tfm_ctx, void *udata, duk_u32 cp) {
	duk_u8 *reserved_table = (duk_u8 *) udata;
	int utf8_blen;
	int min_cp;
	int t;
	int i;

	if (cp == (duk_u32) '%') {
		duk_u8 *p = tfm_ctx->p;
		size_t left = (size_t) (tfm_ctx->p_end - p);  /* bytes left */

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
				duk_hbuffer_append_bytes(tfm_ctx->thr, tfm_ctx->h_buf, (duk_u8 *) (p - 1), 3);
			} else {
				duk_hbuffer_append_byte(tfm_ctx->thr, tfm_ctx->h_buf, (duk_u8) t);
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
			min_cp = 0x80;
			cp = t & 0x1f;
		} else if (t < 0xf0) {
			/* 1110 xxxx; 3 bytes */
			utf8_blen = 3;
			min_cp = 0x800;
			cp = t & 0x0f;
		} else if (t < 0xf8) {
			/* 1111 0xxx; 4 bytes */
			utf8_blen = 4;
			min_cp = 0x10000;
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

		if (cp < min_cp || cp > 0x10ffff || (cp >= 0xd800 && cp <= 0xdfff)) {
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
		DUK_ASSERT(cp >= 0x80 && cp <= 0x10ffff);

		if (cp >= 0x10000) {
			cp -= 0x10000;
			DUK_ASSERT(cp < 0x100000);
			duk_hbuffer_append_xutf8(tfm_ctx->thr, tfm_ctx->h_buf, (cp >> 10) + 0xd800);
			duk_hbuffer_append_xutf8(tfm_ctx->thr, tfm_ctx->h_buf, (cp & 0x03ff) + 0xdc00);
		} else {
			duk_hbuffer_append_xutf8(tfm_ctx->thr, tfm_ctx->h_buf, cp);
		}
	} else {
		duk_hbuffer_append_xutf8(tfm_ctx->thr, tfm_ctx->h_buf, cp);
	}
	return;

 uri_error:
	DUK_ERROR(tfm_ctx->thr, DUK_ERR_URI_ERROR, "invalid input");
}

#ifdef DUK_USE_SECTION_B
static void transform_callback_escape(duk_transform_context *tfm_ctx, void *udata, duk_u32 cp) {
	duk_u8 buf[6];
	size_t len;

	if ((cp < 128) && CHECK_BITMASK(escape_unescaped_table, cp)) {
		buf[0] = (duk_u8) cp;
		len = 1;
	} else if (cp < 256) {
		buf[0] = (duk_u8) '%';
		buf[1] = (duk_u8) duk_uc_nybbles[cp >> 4];
		buf[2] = (duk_u8) duk_uc_nybbles[cp & 0x0f];
		len = 3;
	} else if (cp < 65536) {
		buf[0] = (duk_u8) '%';
		buf[1] = (duk_u8) 'u';
		buf[2] = (duk_u8) duk_uc_nybbles[cp >> 12];
		buf[3] = (duk_u8) duk_uc_nybbles[(cp >> 8) & 0x0f];
		buf[4] = (duk_u8) duk_uc_nybbles[(cp >> 4) & 0x0f];
		buf[5] = (duk_u8) duk_uc_nybbles[cp & 0x0f];
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

static void transform_callback_unescape(duk_transform_context *tfm_ctx, void *udata, duk_u32 cp) {
	int t;

	if (cp == (duk_u32) '%') {
		duk_u8 *p = tfm_ctx->p;
		size_t left = (size_t) (tfm_ctx->p_end - p);  /* bytes left */

		if (left >= 5 && p[0] == 'u' &&
		    ((t = decode_hex_escape(p + 1, 4)) >= 0)) {
			cp = (duk_u32) t;
			tfm_ctx->p += 5;
		} else if (left >= 2 &&
		    ((t = decode_hex_escape(p, 2)) >= 0)) {
			cp = (duk_u32) t;
			tfm_ctx->p += 2;
		}
	}

	duk_hbuffer_append_xutf8(tfm_ctx->thr, tfm_ctx->h_buf, cp);
}

#endif  /* DUK_USE_SECTION_B */

/*
 *  Eval
 */

int duk_builtin_global_object_eval(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_hstring *h;
	duk_activation *act;
	duk_hcompiledfunction *func;
	duk_hobject *outer_lex_env;
	duk_hobject *outer_var_env;
	int this_to_global = 1;
	int comp_flags;

	DUK_ASSERT(duk_get_top(ctx) == 1);

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
	act = thr->callstack + thr->callstack_top - 2;  /* caller */
	if (act->flags & DUK_ACT_FLAG_STRICT) {
		comp_flags |= DUK_JS_COMPILE_FLAG_STRICT;
	}

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

			(void) duk_push_new_object_helper(ctx,
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

int duk_builtin_global_object_parse_int(duk_context *ctx) {
	duk_hstring *h_str;
	duk_u8 *p_start, *p_end, *p, *p_start_dig;
	int neg = 0;
	int strip_prefix;
	duk_i32 radix;
	int t;
	double val;

	duk_to_string(ctx, 0);
	duk_trim(ctx, 0);
	h_str = duk_get_hstring(ctx, 0);
	DUK_ASSERT(h_str != NULL);
	p_start = DUK_HSTRING_GET_DATA(h_str);
	p_end = p_start + DUK_HSTRING_GET_BYTELEN(h_str);
	p = p_start;

	if (p_end > p) {
		t = *p;
		if (t == '-') {
			neg = 1;
			p++;
		} else if (t == '+') {
			p++;
		}
	}

	strip_prefix = 1;
	radix = duk_to_int32(ctx, 1);
	if (radix != 0) {
		if (radix < 2 || radix > 36) {
			goto ret_nan;
		}
		if (radix != 16) {
			strip_prefix = 0;
		}
	} else {
		radix = 10;
	}
	if (strip_prefix) {
		if ((p_end - p >= 2) &&
		    (p[0] == (duk_u8) '0') &&
		    ((p[1] == (duk_u8) 'x') || (p[1] == (duk_u8) 'X'))) {
			p += 2;
			radix = 16;
		}
	}

	/* FIXME: this is correct for radix 2, 4, 8, 16, and 32, but incorrect
	 * for radix 10 which is also required to be "exact".  Other radixes are
	 * not required to be exact, so this would be OK for them.
	 */

	p_start_dig = p;
	val = 0.0;
	while (p < p_end) {
		t = *p;
		if (t >= (int) '0' && t <= (int) '9') {
			t = t - (int) '0';
		} else if (t >= (int) 'a' && t <= (int) 'z') {
			t = t - (int) 'a' + 0x0a;
		} else if (t >= (int) 'A' && t <= (int) 'Z') {
			t = t - (int) 'A' + 0x0a;
		} else {
			break;
		}
		if (t >= radix) {
			break;
		}

		val = val * ((double) radix) + ((double) t);
		p++;
	}
	if (p == p_start_dig) {
		goto ret_nan;
	}
	if (neg) {
		val = -val;
	}

	duk_push_number(ctx, val);
	return 1;

 ret_nan:
	duk_push_nan(ctx);
	return 1;
}

int duk_builtin_global_object_parse_float(duk_context *ctx) {
	/* FIXME: incorrect placeholder */
	duk_to_string(ctx, 0);
	duk_trim(ctx, 0);
	duk_to_number(ctx, 0);
	return 1;
}

/*
 *  Number checkers
 */
int duk_builtin_global_object_is_nan(duk_context *ctx) {
	double d = duk_to_number(ctx, 0);
	duk_push_boolean(ctx, isnan(d));
	return 1;
}

int duk_builtin_global_object_is_finite(duk_context *ctx) {
	double d = duk_to_number(ctx, 0);
	duk_push_boolean(ctx, isfinite(d));
	return 1;
}

/*
 *  URI handling
 */

int duk_builtin_global_object_decode_uri(duk_context *ctx) {
	return transform_helper(ctx, transform_callback_decode_uri, (void *) decode_uri_reserved_table);
}

int duk_builtin_global_object_decode_uri_component(duk_context *ctx) {
	return transform_helper(ctx, transform_callback_decode_uri, (void *) decode_uri_component_reserved_table);
}

int duk_builtin_global_object_encode_uri(duk_context *ctx) {
	return transform_helper(ctx, transform_callback_encode_uri, (void *) encode_uri_unescaped_table);
}

int duk_builtin_global_object_encode_uri_component(duk_context *ctx) {
	return transform_helper(ctx, transform_callback_encode_uri, (void *) encode_uri_component_unescaped_table);
}

#ifdef DUK_USE_SECTION_B
int duk_builtin_global_object_escape(duk_context *ctx) {
	return transform_helper(ctx, transform_callback_escape, (void *) NULL);
}

int duk_builtin_global_object_unescape(duk_context *ctx) {
	return transform_helper(ctx, transform_callback_unescape, (void *) NULL);
}
#endif

#ifdef DUK_USE_BROWSER_LIKE
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
		buf = duk_get_buffer(ctx, 0, &sz);
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

int duk_builtin_global_object_print(duk_context *ctx) {
	return print_alert_helper(ctx, stdout);
}

int duk_builtin_global_object_alert(duk_context *ctx) {
	return print_alert_helper(ctx, stderr);
}
#endif  /* DUK_USE_BROWSER_LIKE */


