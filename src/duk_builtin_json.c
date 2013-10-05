/*
 *  JSON built-ins.
 *
 *  See doc/json.txt.
 */

#include "duk_internal.h"

/*
 *  Local defines and forward declarations.
 */

static void json_dec_syntax_error(duk_json_dec_ctx *js_ctx);
static void json_dec_eat_white(duk_json_dec_ctx *js_ctx);
static int json_dec_peek(duk_json_dec_ctx *js_ctx);
static int json_dec_get(duk_json_dec_ctx *js_ctx);
static int json_dec_get_nonwhite(duk_json_dec_ctx *js_ctx);
static duk_u32 json_dec_decode_hex_escape(duk_json_dec_ctx *js_ctx, int n);
static void json_dec_req_stridx(duk_json_dec_ctx *js_ctx, int stridx);
static void json_dec_string(duk_json_dec_ctx *js_ctx);
static void json_dec_number(duk_json_dec_ctx *js_ctx);
static void json_dec_objarr_shared_entry(duk_json_dec_ctx *js_ctx);
static void json_dec_objarr_shared_exit(duk_json_dec_ctx *js_ctx);
static void json_dec_object(duk_json_dec_ctx *js_ctx);
static void json_dec_array(duk_json_dec_ctx *js_ctx);
static void json_dec_value(duk_json_dec_ctx *js_ctx);
static void json_dec_reviver_walk(duk_json_dec_ctx *js_ctx);

static void json_emit_1(duk_json_enc_ctx *js_ctx, char ch);
static void json_emit_2(duk_json_enc_ctx *js_ctx, int chars);
static void json_emit_esc(duk_json_enc_ctx *js_ctx, duk_u32 cp, char *esc_str, int digits);
static void json_emit_esc16(duk_json_enc_ctx *js_ctx, duk_u32 cp);
static void json_emit_esc32(duk_json_enc_ctx *js_ctx, duk_u32 cp);
static void json_emit_xutf8(duk_json_enc_ctx *js_ctx, duk_u32 cp);
static void json_emit_hstring(duk_json_enc_ctx *js_ctx, duk_hstring *h);
static void json_emit_cstring(duk_json_enc_ctx *js_ctx, const char *p);
static int json_enc_key_quotes_needed(duk_hstring *h_key);
static void json_enc_quote_string(duk_json_enc_ctx *js_ctx, duk_hstring *h_str);
static void json_enc_objarr_shared_entry(duk_json_enc_ctx *js_ctx, duk_hstring **h_stepback, duk_hstring **h_indent, int *entry_top);
static void json_enc_objarr_shared_exit(duk_json_enc_ctx *js_ctx, duk_hstring **h_stepback, duk_hstring **h_indent, int *entry_top);
static void json_enc_object(duk_json_enc_ctx *js_ctx);
static void json_enc_array(duk_json_enc_ctx *js_ctx);
static int json_enc_value1(duk_json_enc_ctx *js_ctx, int idx_holder);
static void json_enc_value2(duk_json_enc_ctx *js_ctx);
static int json_enc_allow_into_proplist(duk_tval *tv);

/*
 *  Parsing implementation.
 *
 *  JSON lexer is now separate from duk_lexer.c because there are numerous
 *  small differences making it difficult to share the lexer.
 *
 *  The parser here works with raw bytes directly; this works because all
 *  JSON delimiters are ASCII characters.  Invalid xUTF-8 encoded values
 *  inside strings will be passed on without normalization; this is not a
 *  compliance concern because compliant inputs will always be valid
 *  CESU-8 encodings.
 */

static void json_dec_syntax_error(duk_json_dec_ctx *js_ctx) {
	/* Shared handler to minimize parser size.  Cause will be
	 * hidden, unfortunately.
	 */
	DUK_ERROR(js_ctx->thr, DUK_ERR_SYNTAX_ERROR, "invalid json");
}

static void json_dec_eat_white(duk_json_dec_ctx *js_ctx) {
	int t;
	for (;;) {
		if (js_ctx->p >= js_ctx->p_end) {
			break;
		}
		t = (int) (*js_ctx->p);
		if (!(t == 0x20 || t == 0x0a || t == 0x0d || t == 0x09)) {
			break;
		}
		js_ctx->p++;
	}
}

static int json_dec_peek(duk_json_dec_ctx *js_ctx) {
	if (js_ctx->p >= js_ctx->p_end) {
		return -1;
	} else {
		return (int) (*js_ctx->p);
	}
}

static int json_dec_get(duk_json_dec_ctx *js_ctx) {
	/* FIXME: multiple EOFs will now be supplied to the caller.  This could also
	 * be changed so that reading the second EOF would cause an error automatically.
	 */
	if (js_ctx->p >= js_ctx->p_end) {
		return -1;
	} else {
		return (int) (*js_ctx->p++);
	}
}

static int json_dec_get_nonwhite(duk_json_dec_ctx *js_ctx) {
	json_dec_eat_white(js_ctx);
	return json_dec_get(js_ctx);
}

static duk_u32 json_dec_decode_hex_escape(duk_json_dec_ctx *js_ctx, int n) {
	int i;
	duk_u32 res = 0;
	int x;

	for (i = 0; i < n; i++) {
		/* FIXME: share helper from lexer; duk_lexer.c / hexval(). */

		x = json_dec_get(js_ctx);

		DUK_DDDPRINT("decode_hex_escape: i=%d, n=%d, res=%d, x=%d",
		             i, n, (int) res, x);

		res *= 16;
		if (x >= (int) '0' && x <= (int) '9') {
			res += x - (int) '0';
		} else if (x >= 'a' && x <= 'f') {
			res += x - (int) 'a' + 0x0a;
		} else if (x >= 'A' && x <= 'F') {
			res += x - (int) 'A' + 0x0a;
		} else {
			/* catches EOF */
			goto syntax_error;
		}
	}

	DUK_DDDPRINT("final hex decoded value: %d", (int) res);
	return res;

 syntax_error:
	json_dec_syntax_error(js_ctx);
	DUK_NEVER_HERE();
	return 0;
}

static void json_dec_req_stridx(duk_json_dec_ctx *js_ctx, int stridx) {
	duk_hstring *h;
	duk_u8 *p;
	duk_u8 *p_end;
	int x;

	/* First character has already been eaten and checked by the
	 * caller.
	 */

	DUK_ASSERT(stridx >= 0 && stridx < DUK_HEAP_NUM_STRINGS);
	h = DUK_HTHREAD_GET_STRING(js_ctx->thr, stridx);
	DUK_ASSERT(h != NULL);

	p = (duk_u8 *) DUK_HSTRING_GET_DATA(h);
	p_end = ((duk_u8 *) DUK_HSTRING_GET_DATA(h)) +
	        DUK_HSTRING_GET_BYTELEN(h);

	DUK_ASSERT((int) *(js_ctx->p - 1) == (int) *p);
	p++;  /* first char */

	while (p < p_end) {
		x = json_dec_get(js_ctx);
		if ((int) (*p) != x) {
			/* catches EOF */
			goto syntax_error;
		}
		p++;
	}

	return;

 syntax_error:
	json_dec_syntax_error(js_ctx);
	DUK_NEVER_HERE();
}

static void json_dec_string(duk_json_dec_ctx *js_ctx) {
	duk_hthread *thr = js_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	duk_hbuffer_dynamic *h_buf;
	int x;

	/* '"' was eaten by caller */

	/* Note that we currently parse -bytes-, not codepoints.
	 * All non-ASCII extended UTF-8 will encode to bytes >= 0x80,
	 * so they'll simply pass through (valid UTF-8 or not).
	 */

	duk_push_dynamic_buffer(ctx, 0);
	h_buf = (duk_hbuffer_dynamic *) duk_get_hbuffer(ctx, -1);
	DUK_ASSERT(h_buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(h_buf));

	for (;;) {
		x = json_dec_get(js_ctx);
		if (x == (int) '"') {
			break;
		} else if (x == (int) '\\') {
			x = json_dec_get(js_ctx);
			switch (x) {
			case '\\': break;
			case '"': break;
			case '/': break;
			case 't': x = 0x09; break;
			case 'n': x = 0x0a; break;
			case 'r': x = 0x0d; break;
			case 'f': x = 0x0c; break;
			case 'b': x = 0x08; break;
			case 'u': {
				x = json_dec_decode_hex_escape(js_ctx, 4);
				break;
			}
#if 0  /* FIXME: custom formats */
			case 'U': {
				if (0) {
					x = json_dec_decode_hex_escape(js_ctx, 8);
				} else {
					goto syntax_error;
				}
				break;
			}
			case 'x': {
				if (0) {
					x = json_dec_decode_hex_escape(js_ctx, 2);
				} else {
					goto syntax_error;
				}
				break;
			}
#endif
			default:
				goto syntax_error;
			}
			duk_hbuffer_append_xutf8(thr, h_buf, (duk_u32) x);
		} else if (x < 0x20) {
			/* catches EOF (-1) */
			goto syntax_error;
		} else {
			duk_hbuffer_append_byte(thr, h_buf, (duk_u8) x);
		}
	}

	duk_to_string(ctx, -1);

	/* [ ... str ] */

	return;

 syntax_error:
	json_dec_syntax_error(js_ctx);
	DUK_NEVER_HERE();
}

static void json_dec_number(duk_json_dec_ctx *js_ctx) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_u8 *p_start;
	int x;
	int s2n_flags;

	DUK_DDDPRINT("parse_number");

	/* Caller has already eaten the first character so backtrack one
	 * byte.  This is correct because the first character is either
	 * '-' or a digit (i.e. an ASCII character).
	 */

	js_ctx->p--;  /* safe */
	p_start = js_ctx->p;

	/* FIXME: this is an approximate parses and way too lenient
	 * (will e.g. parse "1.2.3").  Fix when actual number parsing is
	 * added, and ensure that the number parse can be made to obey
	 * the JSON restrictions.
	 */

	for (;;) {
		x = json_dec_peek(js_ctx);

		DUK_DDDPRINT("parse_number: p_start=%p, p=%p, p_end=%p, x=%d",
		             (void *) p_start, (void *) js_ctx->p,
		             (void *) js_ctx->p_end, x);

		if (!((x >= (int) '0' && x <= (int) '9') ||
		      (x == '.' || x == 'e' || x == 'E' || x == '-'))) {
			break;
		}

		js_ctx->p++;  /* safe, because matched char */
	}

	DUK_ASSERT(js_ctx->p > p_start);
	duk_push_lstring(ctx, (const char *) p_start, (size_t) (js_ctx->p - p_start));

	s2n_flags = DUK_S2N_FLAG_ALLOW_EXP |
	            DUK_S2N_FLAG_ALLOW_MINUS |  /* but don't allow leading plus */
	            DUK_S2N_FLAG_ALLOW_FRAC;

	DUK_DDDPRINT("parse_number: string before parsing: %!T", duk_get_tval(ctx, -1));
	duk_numconv_parse(ctx, 10 /*radix*/, s2n_flags);
	if (duk_is_nan(ctx, -1)) {
		/* FIXME: retcode parse error indicator? */
		DUK_ERROR(js_ctx->thr, DUK_ERR_SYNTAX_ERROR, "invalid number");
	}
	DUK_ASSERT(duk_is_number(ctx, -1));
	DUK_DDDPRINT("parse_number: final number: %!T", duk_get_tval(ctx, -1));

	/* [ ... num ] */
}

static void json_dec_objarr_shared_entry(duk_json_dec_ctx *js_ctx) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_require_stack(ctx, DUK_JSON_DEC_REQSTACK);

	/* c recursion check */

	DUK_ASSERT(js_ctx->recursion_depth >= 0);
	DUK_ASSERT(js_ctx->recursion_depth <= js_ctx->recursion_limit);
	if (js_ctx->recursion_depth >= js_ctx->recursion_limit) {
		DUK_ERROR((duk_hthread *) ctx, DUK_ERR_INTERNAL_ERROR, "recursion limit");
	}
	js_ctx->recursion_depth++;
}

static void json_dec_objarr_shared_exit(duk_json_dec_ctx *js_ctx) {
	/* c recursion check */

	DUK_ASSERT(js_ctx->recursion_depth > 0);
	DUK_ASSERT(js_ctx->recursion_depth <= js_ctx->recursion_limit);
	js_ctx->recursion_depth--;
}

static void json_dec_object(duk_json_dec_ctx *js_ctx) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	int key_count;
	int x;

	DUK_DDDPRINT("parse_object");

	json_dec_objarr_shared_entry(js_ctx);

	duk_push_object(ctx);

	/* Initial '{' has been checked and eaten by caller. */

	key_count = 0;
	for (;;) {
		x = json_dec_get_nonwhite(js_ctx);

		DUK_DDDPRINT("parse_object: obj=%!T, x=%d, key_count=%d",
		           duk_get_tval(ctx, -1), x, key_count);

		/* handle comma and closing brace */

		if (x == (int) ',' && key_count > 0) {
			/* accept comma, expect new value */
			x = json_dec_get_nonwhite(js_ctx);
		} else if (x == (int) '}') {
			/* eat closing brace */
			break;
		} else if (key_count == 0) {
			/* accept anything, expect first value (EOF will be
			 * caught by key parsing below.
			 */
			;
		} else {
			/* catches EOF (and initial comma) */
			goto syntax_error;
		}

		/* parse key and value */

		if (x == (int) '"') {
			json_dec_string(js_ctx);
#if 0  /* FIXME: custom format */
		} else if (0 && ((x >= (int) 'a' && x <= (int) 'z') ||
		                 (x >= (int) 'A' && x <= (int) 'Z') ||
		                 (x == (int) '$' && x == (int) '_'))) {
			/* plain key */
			goto syntax_error;  /* FIXME */
#endif
		} else {
			goto syntax_error;
		}

		/* [ ... obj key ] */

		x = json_dec_get_nonwhite(js_ctx);
		if (x != (int) ':') {
			goto syntax_error;
		}

		json_dec_value(js_ctx);

		/* [ ... obj key val ] */

		duk_put_prop(ctx, -3);

		/* [ ... obj ] */

		key_count++;
	}

	/* [ ... obj ] */

	DUK_DDDPRINT("parse_object: final object is %!T", duk_get_tval(ctx, -1));

	json_dec_objarr_shared_exit(js_ctx);
	return;

 syntax_error:
	json_dec_syntax_error(js_ctx);
	DUK_NEVER_HERE();
}

static void json_dec_array(duk_json_dec_ctx *js_ctx) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	int arr_idx;
	int x;

	DUK_DDDPRINT("parse_array");

	json_dec_objarr_shared_entry(js_ctx);

	duk_push_array(ctx);

	/* Initial '[' has been checked and eaten by caller. */

	arr_idx = 0;
	for (;;) {
		x = json_dec_get_nonwhite(js_ctx);

		DUK_DDDPRINT("parse_array: arr=%!T, x=%d, arr_idx=%d",
		             duk_get_tval(ctx, -1), x, arr_idx);

		/* handle comma and closing bracket */

		if ((x == ',') && (arr_idx != 0)) {
			/* accept comma, expect new value */
			;
		} else if (x == (int) ']') {
			/* eat closing bracket */
			break;
		} else if (arr_idx == 0) {
			/* accept anything, expect first value (EOF will be
			 * caught by json_dec_value() below.
			 */
			js_ctx->p--;  /* backtrack (safe) */
		} else {
			/* catches EOF (and initial comma) */
			goto syntax_error;
		}

		/* parse value */

		json_dec_value(js_ctx);

		/* [ ... arr val ] */

		duk_put_prop_index(ctx, -2, arr_idx);
		arr_idx++;
	}

	/* [ ... arr ] */

	DUK_DDDPRINT("parse_array: final array is %!T", duk_get_tval(ctx, -1));

	json_dec_objarr_shared_exit(js_ctx);
	return;

 syntax_error:
	json_dec_syntax_error(js_ctx);
	DUK_NEVER_HERE();
}

static void json_dec_value(duk_json_dec_ctx *js_ctx) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	int x;

	x = json_dec_get_nonwhite(js_ctx);

	DUK_DDDPRINT("parse_value: initial x=%d", x);

	if (x == (int) '"') {
		json_dec_string(js_ctx);
	} else if ((x >= (int) '0' && x <= (int) '9') || (x == (int) '-')) {
#if 0  /* FIXME: custom format */
		if (XXX && json_dec_peek(js_ctx) == 'I') {
			/* parse -Infinity */
		} else {
			/* parse number */
		}
#endif
		/* We already ate 'x', so json_dec_number() will back up one byte. */
		json_dec_number(js_ctx);
	} else if (x == 't') {
		json_dec_req_stridx(js_ctx, DUK_STRIDX_TRUE);
		duk_push_true(ctx);
	} else if (x == 'f') {
		json_dec_req_stridx(js_ctx, DUK_STRIDX_FALSE);
		duk_push_false(ctx);
	} else if (x == 'n') {
		json_dec_req_stridx(js_ctx, DUK_STRIDX_NULL);
		duk_push_null(ctx);
#if 0  /* FIXME: custom format */
	} else if (XXX && x == 'u') {
		/* parse undefined */
	} else if (XXX && x == 'N') {
		/* parse NaN */
	} else if (XXX && x == 'I') {
		/* parse Infinity */
#endif
	} else if (x == '{') {
		json_dec_object(js_ctx);
	} else if (x == '[') {
		json_dec_array(js_ctx);
	} else {
		/* catches EOF */
		goto syntax_error;
	}

	json_dec_eat_white(js_ctx);

	/* [ ... val ] */
	return;

 syntax_error:
	json_dec_syntax_error(js_ctx);
	DUK_NEVER_HERE();
}

/* Recursive value reviver, implements the Walk() algorithm.  No C recursion
 * check is done here because the initial parsing step will already ensure
 * there is a reasonable limit on C recursion depth and hence object depth.
 */
static void json_dec_reviver_walk(duk_json_dec_ctx *js_ctx) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_hobject *h;
	unsigned int i;  /* FIXME: type */
	unsigned int arr_len;  /* FIXME: type */

	DUK_DDDPRINT("walk: top=%d, holder=%!T, name=%!T",
	             duk_get_top(ctx), duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));

	duk_dup_top(ctx);
	duk_get_prop(ctx, -3);  /* -> [ ... holder name val ] */

	h = duk_get_hobject(ctx, -1);
	if (h != NULL) {
		if (DUK_HOBJECT_GET_CLASS_NUMBER(h) == DUK_HOBJECT_CLASS_ARRAY) {
			arr_len = duk_get_length(ctx, -1);
			for (i = 0; i < arr_len; i++) {
				/* [ ... holder name val ] */

				DUK_DDDPRINT("walk: array, top=%d, i=%d, arr_len=%d, holder=%!T, name=%!T, val=%!T",
				             duk_get_top(ctx), i, arr_len, duk_get_tval(ctx, -3),
				             duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));

				/* FIXME: push_uint_string / push_u32_string */
				duk_dup_top(ctx);
				duk_push_number(ctx, (double) i);
				duk_to_string(ctx, -1);  /* -> [ ... holder name val val ToString(i) ] */
				json_dec_reviver_walk(js_ctx);  /* -> [ ... holder name val new_elem ] */

				if (duk_is_undefined(ctx, -1)) {
					duk_pop(ctx);
					duk_del_prop_index(ctx, -1, i);
				} else {
					duk_put_prop_index(ctx, -2, i);
				}
			}
		} else {
			/* [ ... holder name val ] */
			duk_enum(ctx, -1, DUK_ENUM_OWN_PROPERTIES_ONLY /*flags*/);
			while (duk_next(ctx, -1 /*enum_index*/, 0 /*get_value*/)) {
				DUK_DDDPRINT("walk: object, top=%d, holder=%!T, name=%!T, val=%!T, enum=%!iT, obj_key=%!T",
				             duk_get_top(ctx), duk_get_tval(ctx, -5),
				             duk_get_tval(ctx, -4), duk_get_tval(ctx, -3),
				             duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));

				/* [ ... holder name val enum obj_key ] */
				duk_dup(ctx, -3);
				duk_dup(ctx, -2);

				/* [ ... holder name val enum obj_key val obj_key ] */
				json_dec_reviver_walk(js_ctx);

				/* [ ... holder name val enum obj_key new_elem ] */
				if (duk_is_undefined(ctx, -1)) {
					duk_pop(ctx);
					duk_del_prop(ctx, -3);
				} else {
					duk_put_prop(ctx, -4);
				}
			}
			duk_pop(ctx);  /* pop enum */
		}
	}

	/* [ ... holder name val ] */

	duk_dup(ctx, js_ctx->idx_reviver);
	duk_insert(ctx, -4);  /* -> [ ... reviver holder name val ] */
	duk_call_method(ctx, 2);  /* -> [ ... res ] */

	DUK_DDDPRINT("walk: top=%d, result=%!T", duk_get_top(ctx), duk_get_tval(ctx, -1));
}

/*
 *  Stringify implementation.
 */

#define  EMIT_1(js_ctx,ch)       json_emit_1((js_ctx),(ch))
#define  EMIT_2(js_ctx,ch1,ch2)  json_emit_2((js_ctx),(((int)(ch1)) << 8) + (int)(ch2))
#define  EMIT_ESC16(js_ctx,cp)   json_emit_esc16((js_ctx),(cp))
#define  EMIT_ESC32(js_ctx,cp)   json_emit_esc32((js_ctx),(cp))
#define  EMIT_XUTF8(js_ctx,cp)   json_emit_xutf8((js_ctx),(cp))
#define  EMIT_HSTR(js_ctx,h)     json_emit_hstring((js_ctx),(h))
#define  EMIT_CSTR(js_ctx,p)     json_emit_cstring((js_ctx),(p))
#define  EMIT_STRIDX(js_ctx,i)   json_emit_stridx((js_ctx),(i))

static void json_emit_1(duk_json_enc_ctx *js_ctx, char ch) {
	duk_hbuffer_append_byte(js_ctx->thr, js_ctx->h_buf, (duk_u8) ch);
}

static void json_emit_2(duk_json_enc_ctx *js_ctx, int chars) {
	duk_u8 buf[2];
	buf[0] = (chars >> 8);
	buf[1] = chars & 0xff;
	duk_hbuffer_append_bytes(js_ctx->thr, js_ctx->h_buf, (duk_u8 *) buf, 2);
}

static void json_emit_esc(duk_json_enc_ctx *js_ctx, duk_u32 cp, char *esc_str, int digits) {
	int dig;

	duk_hbuffer_append_cstring(js_ctx->thr, js_ctx->h_buf, esc_str);

	while (digits > 0) {
		digits--;
		dig = (cp >> (4 * digits)) & 0x0f;
		duk_hbuffer_append_byte(js_ctx->thr, js_ctx->h_buf, duk_lc_digits[dig]);
	}
}

static void json_emit_esc16(duk_json_enc_ctx *js_ctx, duk_u32 cp) {
	json_emit_esc(js_ctx, cp, "\\u", 4);
}

static void json_emit_esc32(duk_json_enc_ctx *js_ctx, duk_u32 cp) {
	/* custom format */
	json_emit_esc(js_ctx, cp, "\\U", 8);
}

static void json_emit_xutf8(duk_json_enc_ctx *js_ctx, duk_u32 cp) {
	(void) duk_hbuffer_append_xutf8(js_ctx->thr, js_ctx->h_buf, cp);
}

static void json_emit_hstring(duk_json_enc_ctx *js_ctx, duk_hstring *h) {
	DUK_ASSERT(h != NULL);
	duk_hbuffer_append_bytes(js_ctx->thr,
	                         js_ctx->h_buf,
	                         (duk_u8 *) DUK_HSTRING_GET_DATA(h),
	                         (size_t) DUK_HSTRING_GET_BYTELEN(h));
}

static void json_emit_cstring(duk_json_enc_ctx *js_ctx, const char *p) {
	DUK_ASSERT(p != NULL);
	(void) duk_hbuffer_append_cstring(js_ctx->thr, js_ctx->h_buf, p);
}

static void json_emit_stridx(duk_json_enc_ctx *js_ctx, int stridx) {
	DUK_ASSERT(stridx >= 0 && stridx < DUK_HEAP_NUM_STRINGS);
	json_emit_hstring(js_ctx, DUK_HTHREAD_GET_STRING(js_ctx->thr, stridx));
}

/* Check whether key quotes would be needed (custom encoding). */
static int json_enc_key_quotes_needed(duk_hstring *h_key) {
	duk_u8 *p, *p_start, *p_end;
	int ch;

	DUK_ASSERT(h_key != NULL);
	p_start = DUK_HSTRING_GET_DATA(h_key);
	p_end = p_start + DUK_HSTRING_GET_BYTELEN(h_key);
	p = p_start;

	DUK_DDDPRINT("json_enc_key_quotes_needed: h_key=%!O, p_start=%p, p_end=%p, p=%p",
	             (duk_heaphdr *) h_key, (void *) p_start, (void *) p_end, (void *) p);

	/* Since we only accept ASCII characters, there is no need for
	 * actual decoding.  A non-ASCII character will be >= 0x80 which
	 * causes a false return value immediately.
	 */

	while (p < p_end) {
		ch = (int) (*p);

		/* accept ASCII IdentifierStart and IdentifierPart if not first char */
		if ((ch >= (int) 'a' && ch <= (int) 'z') ||
		    (ch >= (int) 'A' && ch <= (int) 'Z') ||
		    (ch == (int) '$' || ch == (int) '_') ||
		    ((p > p_start) && (ch >= (int) '0' && ch <= (int) '9'))) {
			p++;
			continue;
		}

		/* all non-ASCII characters also come here (first byte >= 0x80) */
		return 1;
	}

	return 0;
}

/* The Quote(value) operation: quote a string.
 *
 * Stack policy: [ ] -> [ ].
 */

static char quote_esc[14] = {
	'\0', '\0', '\0', '\0', '\0', '\0', '\0', '\0',
	'b',  't',  'n',  '\0', 'f',  'r'
};

static void json_enc_quote_string(duk_json_enc_ctx *js_ctx, duk_hstring *h_str) {
	duk_hthread *thr = js_ctx->thr;
	duk_u8 *p, *p_start, *p_end;
	duk_u32 cp;

	DUK_DDDPRINT("json_enc_quote_string: h_str=%!O", h_str);

	DUK_ASSERT(h_str != NULL);
	p_start = DUK_HSTRING_GET_DATA(h_str);
	p_end = p_start + DUK_HSTRING_GET_BYTELEN(h_str);
	p = p_start;

	EMIT_1(js_ctx, '"');

	while (p < p_end) {
		cp = *p;

		if (cp <= 0x7f) {
			/* ascii fast path: avoid decoding utf-8 */
			p++;
			if (cp == 0x22 || cp == 0x5c) {
				/* double quote or backslash */
				EMIT_2(js_ctx, '\\', (char) cp);
			} else if (cp < 0x20) {
				char esc_char;

				/* This approach is a bit shorter than a straight
				 * if-else-ladder and also a bit faster.
				 */
				if (cp < sizeof(quote_esc) &&
				    (esc_char = quote_esc[cp]) != (char) 0) {
					EMIT_2(js_ctx, '\\', esc_char);
				} else {
					EMIT_ESC16(js_ctx, cp);
				}
			} else if (cp == 0x7f && js_ctx->flag_ascii_only) {
				EMIT_ESC16(js_ctx, cp);
			} else {
				/* any other printable -> as is */
				EMIT_1(js_ctx, (char) cp);
			}
		} else {
			/* slow path decode */

			/* FIXME: this may currently fail, we'd prefer it never do that */
			cp = duk_unicode_xutf8_get_u32_checked(thr, &p, p_start, p_end);

			if (js_ctx->flag_ascii_only) {
				if (cp > 0xffff) {
					EMIT_ESC32(js_ctx, cp);
				} else {
					EMIT_ESC16(js_ctx, cp);
				}
			} else {
				/* as is */
				EMIT_XUTF8(js_ctx, cp);
			}
		}
	}

	EMIT_1(js_ctx, '"');
}

/* Shared entry handling for object/array serialization: indent/stepback,
 * loop detection.
 */
static void json_enc_objarr_shared_entry(duk_json_enc_ctx *js_ctx, duk_hstring **h_stepback, duk_hstring **h_indent, int *entry_top) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_hobject *h_target;

	*entry_top = duk_get_top(ctx);

	duk_require_stack(ctx, DUK_JSON_ENC_REQSTACK);

	/* loop check */

	h_target = duk_get_hobject(ctx, -1);  /* object or array */
	DUK_ASSERT(h_target != NULL);
	duk_push_sprintf(ctx, "%p", (void *) h_target);

	duk_dup_top(ctx);  /* -> [ ... voidp voidp ] */
	if (duk_has_prop(ctx, js_ctx->idx_loop)) {
		DUK_ERROR((duk_hthread *) ctx, DUK_ERR_TYPE_ERROR, "cyclic input");
	}
	duk_push_true(ctx);  /* -> [ ... voidp true ] */
	duk_put_prop(ctx, js_ctx->idx_loop);  /* -> [ ... ] */

	/* c recursion check */

	DUK_ASSERT(js_ctx->recursion_depth >= 0);
	DUK_ASSERT(js_ctx->recursion_depth <= js_ctx->recursion_limit);
	if (js_ctx->recursion_depth >= js_ctx->recursion_limit) {
		DUK_ERROR((duk_hthread *) ctx, DUK_ERR_INTERNAL_ERROR, "recursion limit");
	}
	js_ctx->recursion_depth++;

	/* figure out indent and stepback */

	*h_indent = NULL;
	*h_stepback = NULL;
	if (js_ctx->h_gap != NULL) {
		DUK_ASSERT(js_ctx->h_indent != NULL);

		*h_stepback = js_ctx->h_indent;
		duk_push_hstring(ctx, js_ctx->h_indent);
		duk_push_hstring(ctx, js_ctx->h_gap);
		duk_concat(ctx, 2);
		js_ctx->h_indent = duk_get_hstring(ctx, -1);
		*h_indent = js_ctx->h_indent;
		DUK_ASSERT(js_ctx->h_indent != NULL);

		/* The new indent string is left at value stack top, and will
		 * be popped by the shared exit handler.
	 	 */
	} else {
		DUK_ASSERT(js_ctx->h_indent == NULL);
	}

	DUK_DDDPRINT("shared entry finished: top=%d, loop=%!T",
	             duk_get_top(ctx), duk_get_tval(ctx, js_ctx->idx_loop));
}

/* Shared exit handling for object/array serialization. */
static void json_enc_objarr_shared_exit(duk_json_enc_ctx *js_ctx, duk_hstring **h_stepback, duk_hstring **h_indent, int *entry_top) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_hobject *h_target;

	if (js_ctx->h_gap != NULL) {
		DUK_ASSERT(js_ctx->h_indent != NULL);
		DUK_ASSERT(*h_stepback != NULL);
		DUK_ASSERT(*h_indent != NULL);

		js_ctx->h_indent = *h_stepback;  /* previous js_ctx->h_indent */

		/* Note: we don't need to pop anything because the duk_set_top()
		 * at the end will take care of it.
		 */
	} else {
		DUK_ASSERT(js_ctx->h_indent == NULL);
		DUK_ASSERT(*h_stepback == NULL);
		DUK_ASSERT(*h_indent == NULL);
	}

	/* c recursion check */

	DUK_ASSERT(js_ctx->recursion_depth > 0);
	DUK_ASSERT(js_ctx->recursion_depth <= js_ctx->recursion_limit);
	js_ctx->recursion_depth--;

	/* loop check */

	h_target = duk_get_hobject(ctx, *entry_top - 1);  /* original target at entry_top - 1 */
	DUK_ASSERT(h_target != NULL);
	duk_push_sprintf(ctx, "%p", (void *) h_target);

	duk_del_prop(ctx, js_ctx->idx_loop);  /* -> [ ... ] */

	/* restore stack top after unbalanced code paths */
	duk_set_top(ctx, *entry_top);

	DUK_DDDPRINT("shared entry finished: top=%d, loop=%!T",
	             duk_get_top(ctx), duk_get_tval(ctx, js_ctx->idx_loop));
}

/* The JO(value) operation: encode object.
 *
 * Stack policy: [ object ] -> [ object ].
 */
static void json_enc_object(duk_json_enc_ctx *js_ctx) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_hstring *h_stepback;
	duk_hstring *h_indent;
	duk_hstring *h_key;
	int entry_top;
	int idx_obj;
	int idx_keys;
	int first;
	int undef;
	int arr_len;
	int i;

	DUK_DDDPRINT("json_enc_object: obj=%!T", duk_get_tval(ctx, -1));

	json_enc_objarr_shared_entry(js_ctx, &h_stepback, &h_indent, &entry_top);

	idx_obj = entry_top - 1;

	if (js_ctx->idx_proplist >= 0) {
		idx_keys = js_ctx->idx_proplist;
	} else {
		/* FIXME: would be nice to enumerate an object at specified index */
		duk_dup(ctx, idx_obj);
		(void) duk_hobject_get_enumerated_keys(ctx, DUK_ENUM_OWN_PROPERTIES_ONLY /*flags*/);  /* [ ... target ] -> [ ... target keys ] */
		idx_keys = duk_require_normalize_index(ctx, -1);
		/* leave stack unbalanced on purpose */
	}

	DUK_DDDPRINT("idx_keys=%d, h_keys=%!T", idx_keys, duk_get_tval(ctx, idx_keys));

	/* Steps 8-10 have been merged to avoid a "partial" variable. */

	EMIT_1(js_ctx, '{');

	/* FIXME: keys is an internal object with all keys to be processed
	 * in its (gapless) array part.  Because nobody can touch the keys
	 * object, we could iterate its array part directly (keeping in mind
	 * that it can be reallocated).
	 */

	arr_len = duk_get_length(ctx, idx_keys);
	first = 1;
	for (i = 0; i < arr_len; i++) {
		duk_get_prop_index(ctx, idx_keys, i);  /* -> [ ... key ] */

		DUK_DDDPRINT("object property loop: holder=%!T, key=%!T",
		             duk_get_tval(ctx, idx_obj), duk_get_tval(ctx, -1));

		undef = json_enc_value1(js_ctx, idx_obj);
		if (undef) {
			/* Value would yield 'undefined', so skip key altogether.
			 * Side effects have already happened.
			 */
			continue;
		}

		/* [ ... key val ] */

		if (first) {
			first = 0;
		} else {
			EMIT_1(js_ctx, (char) ',');
		}
		if (h_indent != NULL) {
			EMIT_1(js_ctx, (char) 0x0a);
			EMIT_HSTR(js_ctx, h_indent);
		}

		h_key = duk_get_hstring(ctx, -2);
		DUK_ASSERT(h_key != NULL);
		if (js_ctx->flag_avoid_key_quotes && !json_enc_key_quotes_needed(h_key)) {
			/* emit key as is */
			EMIT_HSTR(js_ctx, h_key);
		} else {
			json_enc_quote_string(js_ctx, h_key);
		}

		if (h_indent != NULL) {
			EMIT_2(js_ctx, ':', ' ');
		} else {
			EMIT_1(js_ctx, ':');
		}

		/* [ ... key val ] */

		json_enc_value2(js_ctx);  /* -> [ ... ] */
	}

	if (!first) {
		if (h_stepback != NULL) {
			DUK_ASSERT(h_indent != NULL);
			EMIT_1(js_ctx, (char) 0x0a);
			EMIT_HSTR(js_ctx, h_stepback);
		}
	}
	EMIT_1(js_ctx, '}');

	json_enc_objarr_shared_exit(js_ctx, &h_stepback, &h_indent, &entry_top);

	DUK_ASSERT_TOP(ctx, entry_top);
}

/* The JA(value) operation: encode array.
 *
 * Stack policy: [ array ] -> [ array ].
 */
static void json_enc_array(duk_json_enc_ctx *js_ctx) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_hstring *h_stepback;
	duk_hstring *h_indent;
	int entry_top;
	int idx_arr;
	int undef;
	unsigned int i;
	unsigned int arr_len;  /* FIXME: type */

	DUK_DDDPRINT("json_enc_array: array=%!T", duk_get_tval(ctx, -1));

	json_enc_objarr_shared_entry(js_ctx, &h_stepback, &h_indent, &entry_top);

	idx_arr = entry_top - 1;

	/* Steps 8-10 have been merged to avoid a "partial" variable. */

	EMIT_1(js_ctx, '[');

	arr_len = duk_get_length(ctx, idx_arr);
	for (i = 0; i < arr_len; i++) {
		DUK_DDDPRINT("array entry loop: array=%!T, h_indent=%!O, h_stepback=%!O, index=%d, arr_len=%d",
		             duk_get_tval(ctx, idx_arr), h_indent, h_stepback, i, arr_len);

		if (i > 0) {
			EMIT_1(js_ctx, ',');
		}
		if (h_indent != NULL) {
			EMIT_1(js_ctx, (char) 0x0a);
			EMIT_HSTR(js_ctx, h_indent);
		}

		/* FIXME: duk_push_uint_string() */
		duk_push_number(ctx, (double) i);
		duk_to_string(ctx, -1);  /* -> [ ... key ] */
		undef = json_enc_value1(js_ctx, idx_arr);

		if (undef) {
			EMIT_STRIDX(js_ctx, DUK_STRIDX_NULL);
		} else {
			/* [ ... key val ] */
			json_enc_value2(js_ctx);
		}
	}

	if (arr_len > 0) {
		if (h_stepback != NULL) {
			DUK_ASSERT(h_indent != NULL);
			EMIT_1(js_ctx, (char) 0x0a);
			EMIT_HSTR(js_ctx, h_stepback);
		}
	}
	EMIT_1(js_ctx, ']');

	json_enc_objarr_shared_exit(js_ctx, &h_stepback, &h_indent, &entry_top);

	DUK_ASSERT_TOP(ctx, entry_top);
}

/* The Str(key, holder) operation: encode value, steps 1-4.
 *
 * Returns non-zero if the value between steps 4 and 5 would yield an
 * 'undefined' final result.  This is useful in JO() because we need to
 * get the side effects out, but need to know whether or not a key will
 * be omitted from the serialization.
 *
 * Stack policy: [ ... key ] -> [ ... key val ]  if retval == 0.
 *                           -> [ ... ]          if retval != 0.
 */
static int json_enc_value1(duk_json_enc_ctx *js_ctx, int idx_holder) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_hobject *h;
	duk_tval *tv;
	int c;

	DUK_DDDPRINT("json_enc_value1: idx_holder=%d, holder=%!T, key=%!T",
	             idx_holder, duk_get_tval(ctx, idx_holder), duk_get_tval(ctx, -1));

	duk_dup_top(ctx);               /* -> [ ... key key ] */
	duk_get_prop(ctx, idx_holder);  /* -> [ ... key val ] */

	DUK_DDDPRINT("value=%!T", duk_get_tval(ctx, -1));

	h = duk_get_hobject(ctx, -1);
	if (h != NULL) {
		duk_get_prop_stridx(ctx, -1, DUK_STRIDX_TO_JSON);
		h = duk_get_hobject(ctx, -1);  /* FIXME: duk_get_hobject_callable */
		if (h != NULL && DUK_HOBJECT_IS_CALLABLE(h)) {
			DUK_DDDPRINT("value is object, has callable toJSON() -> call it");
			duk_dup(ctx, -2);         /* -> [ ... key val toJSON val ] */
			duk_dup(ctx, -4);         /* -> [ ... key val toJSON val key ] */
			duk_call_method(ctx, 1);  /* -> [ ... key val val' ] */
			duk_remove(ctx, -2);      /* -> [ ... key val' ] */
		} else {
			duk_pop(ctx);
		}
	}

	/* [ ... key val ] */

	DUK_DDDPRINT("value=%!T", duk_get_tval(ctx, -1));

	if (js_ctx->h_replacer) {
		/* FIXME: here a "slice copy" would be useful */
		DUK_DDDPRINT("replacer is set, call replacer");
		duk_push_hobject(ctx, js_ctx->h_replacer);  /* -> [ ... key val replacer ] */
		duk_dup(ctx, idx_holder);                   /* -> [ ... key val replacer holder ] */
		duk_dup(ctx, -4);                           /* -> [ ... key val replacer holder key ] */
		duk_dup(ctx, -4);                           /* -> [ ... key val replacer holder key val ] */
		duk_call_method(ctx, 2);                    /* -> [ ... key val val' ] */
		duk_remove(ctx, -2);                        /* -> [ ... key val' ] */
	}

	/* [ ... key val ] */

	DUK_DDDPRINT("value=%!T", duk_get_tval(ctx, -1));

	tv = duk_get_tval(ctx, -1);
	DUK_ASSERT(tv != NULL);
	if (DUK_TVAL_IS_OBJECT(tv)) {
		h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);

		c = DUK_HOBJECT_GET_CLASS_NUMBER(h);
		if (c == DUK_HOBJECT_CLASS_NUMBER) {
			DUK_DDDPRINT("value is a Number object -> coerce with ToNumber()");
			duk_to_number(ctx, -1);
		} else if (c == DUK_HOBJECT_CLASS_STRING) {
			DUK_DDDPRINT("value is a String object -> coerce with ToString()");
			duk_to_string(ctx, -1);
		} else if (c == DUK_HOBJECT_CLASS_BOOLEAN) {
			DUK_DDDPRINT("value is a Boolean object -> get internal value");
			duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_VALUE);
			DUK_ASSERT(DUK_TVAL_IS_BOOLEAN(duk_get_tval(ctx, -1)));
			duk_remove(ctx, -2);
		}
	}

	/* [ ... key val ] */

	DUK_DDDPRINT("value=%!T", duk_get_tval(ctx, -1));

	tv = duk_get_tval(ctx, -1);
	DUK_ASSERT(tv != NULL);

	if (duk_check_type_mask(ctx, -1, js_ctx->mask_for_undefined)) {
		/* will result in undefined */
		DUK_DDDPRINT("-> will result in undefined (type mask check)");
		goto undef;
	}

	h = duk_get_hobject(ctx, -1);
	if (h != NULL && DUK_HOBJECT_IS_CALLABLE(h)) {
		if (js_ctx->flags & (DUK_JSON_ENC_FLAG_EXT_CUSTOM |
		                     DUK_JSON_ENC_FLAG_EXT_COMPATIBLE)) {
			/* function will be serialized to custom format */
		} else {
			/* functions are not serialized, results in undefined */
			DUK_DDDPRINT("-> will result in undefined (function)");
			goto undef;
		}
	}

	DUK_DDDPRINT("-> will not result in undefined");
	return 0;

 undef:
	duk_pop_2(ctx);
	return 1;
}

/* The Str(key, holder) operation: encode value, steps 5-10.
 *
 * This must not be called unless json_enc_value1() returns non-zero.
 * If so, this is guaranteed to produce a non-undefined result.
 * Non-standard encodings (e.g. for undefined) are only used if
 * json_enc_value1() indicates they are accepted; they're not
 * checked or asserted here again.
 *
 * Stack policy: [ ... key val ] -> [ ... ].
 */
static void json_enc_value2(duk_json_enc_ctx *js_ctx) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_tval *tv;

	DUK_DDDPRINT("json_enc_value2: key=%!T, val=%!T",
	             duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));

	/* [ ... key val ] */

	tv = duk_get_tval(ctx, -1);
	DUK_ASSERT(tv != NULL);

	switch (DUK_TVAL_GET_TAG(tv)) {
	case DUK_TAG_UNDEFINED: {
		EMIT_STRIDX(js_ctx, js_ctx->stridx_custom_undefined);
		break;
	}
	case DUK_TAG_NULL: {
		EMIT_STRIDX(js_ctx, DUK_STRIDX_NULL);
		break;
	}
	case DUK_TAG_BOOLEAN: {
		EMIT_STRIDX(js_ctx, DUK_TVAL_GET_BOOLEAN(tv) ?
		            DUK_STRIDX_TRUE : DUK_STRIDX_FALSE);
		break;
	}
	case DUK_TAG_POINTER: {
		/* FIXME: custom format unfinished */
		char buf[40];  /* FIXME: how to figure correct size? */
		const char *fmt;

		/* FIXME: NULL results in '((nil))' now */
		DUK_MEMSET(buf, 0, sizeof(buf));
		if (js_ctx->flag_ext_custom) {
			fmt = "(%p)";
		} else {
			DUK_ASSERT(js_ctx->flag_ext_compatible);
			fmt = "{\"_ptr\":\"(%p)\"}";
		}
		DUK_SNPRINTF(buf, sizeof(buf) - 1, fmt, (void *) DUK_TVAL_GET_POINTER(tv));
		EMIT_CSTR(js_ctx, buf);
		break;
	}
	case DUK_TAG_STRING: {
		duk_hstring *h = DUK_TVAL_GET_STRING(tv);
		DUK_ASSERT(h != NULL);

		json_enc_quote_string(js_ctx, h);
		break;
	}
	case DUK_TAG_OBJECT: {
		duk_hobject *h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);

		if (DUK_HOBJECT_IS_CALLABLE(h)) {
			if (js_ctx->flag_ext_custom) {
				/* FIXME: just ToString() now */
			} else {
				DUK_ASSERT(js_ctx->flag_ext_compatible);
				/* FIXME: just ToString() now */
			}
			duk_to_string(ctx, -1);
			json_enc_quote_string(js_ctx, duk_require_hstring(ctx, -1));
		} else if (DUK_HOBJECT_GET_CLASS_NUMBER(h) == DUK_HOBJECT_CLASS_ARRAY) {
			json_enc_array(js_ctx);
		} else {
			json_enc_object(js_ctx);
		}
		break;
	}
	case DUK_TAG_BUFFER: {
		duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);
		DUK_ASSERT(h != NULL);

		/* FIXME: custom format unfinished */

		/* FIXME: other alternatives for binary include base64, base85,
		 * encoding as Unicode 8-bit codepoints, etc.
		 */

		if (js_ctx->flag_ext_custom) {
			duk_u8 *p, *p_end;
			int x;
			p = (duk_u8 *) DUK_HBUFFER_GET_DATA_PTR(h);
			p_end = p + DUK_HBUFFER_GET_SIZE(h);
			EMIT_1(js_ctx, '|');
			while (p < p_end) {
				x = (int) *p++;
				duk_hbuffer_append_byte(js_ctx->thr, js_ctx->h_buf, duk_lc_digits[(x >> 4) & 0x0f]);
				duk_hbuffer_append_byte(js_ctx->thr, js_ctx->h_buf, duk_lc_digits[x & 0x0f]);
			}
			EMIT_1(js_ctx, '|');
		} else {
			DUK_ASSERT(js_ctx->flag_ext_compatible);
			duk_base64_encode(ctx, -1);
			EMIT_CSTR(js_ctx, "{\"_base64\":");  /* FIXME: stridx */
			json_enc_quote_string(js_ctx, duk_require_hstring(ctx, -1));
			EMIT_1(js_ctx, '}');
		}
		break;
	}
	default: {
		/* number */
		double d;
		int c;
		int s;
		int stridx;
		int n2s_flags;
		duk_hstring *h_str;
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));

		d = DUK_TVAL_GET_NUMBER(tv);
		c = DUK_FPCLASSIFY(d);
		s = DUK_SIGNBIT(d);

		if (!(c == DUK_FP_INFINITE || c == DUK_FP_NAN)) {
			DUK_ASSERT(DUK_ISFINITE(d));
			n2s_flags = 0;
			/* [ ... number ] -> [ ... string ] */
			duk_numconv_stringify(ctx, 10 /*radix*/, 0 /*digits*/, n2s_flags);
			h_str = duk_to_hstring(ctx, -1);
			DUK_ASSERT(h_str != NULL);
			EMIT_HSTR(js_ctx, h_str);
			break;
		}

		/* FIXME: awkward check */

		if (!(js_ctx->flags & (DUK_JSON_ENC_FLAG_EXT_CUSTOM |
		                       DUK_JSON_ENC_FLAG_EXT_COMPATIBLE))) {
			stridx = DUK_STRIDX_NULL;
		} else if (c == DUK_FP_NAN) {
			stridx = js_ctx->stridx_custom_nan;
		} else if (s == 0) {
			stridx = js_ctx->stridx_custom_neginf;
		} else {
			stridx = js_ctx->stridx_custom_posinf;
		}
		EMIT_STRIDX(js_ctx, stridx);
		break;
	}
	}

	/* [ ... key val ] -> [ ... ] */

	duk_pop_2(ctx);
}

/* E5 Section 15.12.3, main algorithm, step 4.b.ii steps 1-4. */
static int json_enc_allow_into_proplist(duk_tval *tv) {
	duk_hobject *h;
	int c;

	DUK_ASSERT(tv != NULL);
	if (DUK_TVAL_IS_STRING(tv) || DUK_TVAL_IS_NUMBER(tv)) {
		return 1;
	} else if (DUK_TVAL_IS_OBJECT(tv)) {
		h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);
		c = DUK_HOBJECT_GET_CLASS_NUMBER(h);
		if (c == DUK_HOBJECT_CLASS_STRING || c == DUK_HOBJECT_CLASS_NUMBER) {
			return 1;
		}
	}

	return 0;
}

/*
 *  Top level wrappers
 */

void duk_builtin_json_parse_helper(duk_context *ctx,
                                   int idx_value,
                                   int idx_reviver,
                                   int flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_json_dec_ctx js_ctx_alloc;
	duk_json_dec_ctx *js_ctx = &js_ctx_alloc;
	duk_hstring *h_text;
#ifdef DUK_USE_ASSERTIONS
	int top_at_entry = duk_get_top(ctx);
#endif

	DUK_DDDPRINT("JSON parse start: text=%!T, reviver=%!T, flags=0x%08x, stack_top=%d",
	             duk_get_tval(ctx, idx_value), duk_get_tval(ctx, idx_reviver),
	             flags, duk_get_top(ctx));

	DUK_MEMSET(&js_ctx_alloc, 0, sizeof(js_ctx_alloc));
	js_ctx->thr = thr;
#ifdef DUK_USE_EXPLICIT_NULL_INIT
	/* nothing now */
#endif
	js_ctx->recursion_limit = DUK_JSON_DEC_RECURSION_LIMIT;

	/* FIXME: flags for custom parsing */

	h_text = duk_to_hstring(ctx, idx_value);  /* coerce in-place */
	DUK_ASSERT(h_text != NULL);

	js_ctx->p = (duk_u8 *) DUK_HSTRING_GET_DATA(h_text);
	js_ctx->p_end = ((duk_u8 *) DUK_HSTRING_GET_DATA(h_text)) +
	                DUK_HSTRING_GET_BYTELEN(h_text);

	json_dec_value(js_ctx);  /* -> [ ... value ] */

	/* Trailing whitespace has been eaten by json_dec_value(), so if
	 * we're not at end of input here, it's a SyntaxError.
	 */

	if (js_ctx->p != js_ctx->p_end) {
		DUK_ERROR(thr, DUK_ERR_SYNTAX_ERROR, "invalid json");
	}

	if (duk_is_callable(ctx, idx_reviver)) {
		DUK_DDDPRINT("applying reviver: %!T", duk_get_tval(ctx, idx_reviver));

		js_ctx->idx_reviver = idx_reviver;

		DUK_ASSERT_TOP(ctx, 3);

		duk_push_object(ctx);
		duk_dup(ctx, -2);  /* -> [ ... val root val ] */
		duk_put_prop_stridx(ctx, -2, DUK_STRIDX_EMPTY_STRING);  /* default attrs ok */
		duk_push_hstring_stridx(ctx, DUK_STRIDX_EMPTY_STRING);  /* -> [ ... val root "" ] */

		DUK_DDDPRINT("start reviver walk, root=%!T, name=%!T",
		             duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));

		json_dec_reviver_walk(js_ctx);  /* [ ... val root "" ] -> [ ... val val' ] */
		duk_remove(ctx, -2);            /* -> [ ... val' ] */
	} else {
		DUK_DDDPRINT("reviver does not exist or is not callable: %!T",
		             duk_get_tval(ctx, idx_reviver));
	}

	/* Final result is at stack top. */

	DUK_DDDPRINT("JSON parse end: text=%!T, reviver=%!T, flags=0x%08x, result=%!T, stack_top=%d",
	             duk_get_tval(ctx, idx_value), duk_get_tval(ctx, idx_reviver),
	             flags, duk_get_tval(ctx, -1), duk_get_top(ctx));

	DUK_ASSERT(duk_get_top(ctx) == top_at_entry + 1);
}

void duk_builtin_json_stringify_helper(duk_context *ctx,
                                       int idx_value,
                                       int idx_replacer,
                                       int idx_space,
                                       int flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_json_enc_ctx js_ctx_alloc;
	duk_json_enc_ctx *js_ctx = &js_ctx_alloc;
	duk_hobject *h;
	int undef;
	int idx_holder;
	int top_at_entry;

	DUK_DDDPRINT("JSON stringify start: value=%!T, replacer=%!T, space=%!T, flags=0x%08x, stack_top=%d",
	             duk_get_tval(ctx, idx_value), duk_get_tval(ctx, idx_replacer),
	             duk_get_tval(ctx, idx_space), flags, duk_get_top(ctx));

	top_at_entry = duk_get_top(ctx);

	/*
	 *  Context init
	 */

	DUK_MEMSET(&js_ctx_alloc, 0, sizeof(js_ctx_alloc));
	js_ctx->thr = thr;
#ifdef DUK_USE_EXPLICIT_NULL_INIT
	js_ctx->h_replacer = NULL;
	js_ctx->h_gap = NULL;
	js_ctx->h_indent = NULL;
#endif
	js_ctx->idx_proplist = -1;
	js_ctx->recursion_limit = DUK_JSON_ENC_RECURSION_LIMIT;

#if 0  /* FIXME: custom formats; some implemented, some not */
	flags |= DUK_JSON_ENC_FLAG_ASCII_ONLY;
	flags |= DUK_JSON_ENC_FLAG_AVOID_KEY_QUOTES;
	flags |= DUK_JSON_ENC_FLAG_EXT_COMPATIBLE;
#endif
	js_ctx->flags = flags;
	js_ctx->flag_ascii_only = flags & DUK_JSON_ENC_FLAG_ASCII_ONLY;
	js_ctx->flag_avoid_key_quotes = flags & DUK_JSON_ENC_FLAG_AVOID_KEY_QUOTES;
	js_ctx->flag_ext_custom = flags & DUK_JSON_ENC_FLAG_EXT_CUSTOM;
	js_ctx->flag_ext_compatible = flags & DUK_JSON_ENC_FLAG_EXT_COMPATIBLE;

	if (flags & DUK_JSON_ENC_FLAG_EXT_CUSTOM) {
		js_ctx->stridx_custom_undefined = DUK_STRIDX_UNDEFINED;
		js_ctx->stridx_custom_nan = DUK_STRIDX_NAN;
		js_ctx->stridx_custom_neginf = DUK_STRIDX_MINUS_INFINITY;
		js_ctx->stridx_custom_posinf = DUK_STRIDX_INFINITY;
	} else if (js_ctx->flags & DUK_JSON_ENC_FLAG_EXT_COMPATIBLE) {
		js_ctx->stridx_custom_undefined = DUK_STRIDX_JSON_EXT_UNDEFINED;
		js_ctx->stridx_custom_nan = DUK_STRIDX_JSON_EXT_NAN;
		js_ctx->stridx_custom_neginf = DUK_STRIDX_JSON_EXT_NEGINF;
		js_ctx->stridx_custom_posinf = DUK_STRIDX_JSON_EXT_POSINF;
	}

	if (js_ctx->flags & (DUK_JSON_ENC_FLAG_EXT_CUSTOM |
	                     DUK_JSON_ENC_FLAG_EXT_COMPATIBLE)) {
		DUK_ASSERT(js_ctx->mask_for_undefined == 0);  /* already zero */
	} else {
		js_ctx->mask_for_undefined = DUK_TYPE_MASK_UNDEFINED |
		                             DUK_TYPE_MASK_POINTER |
		                             DUK_TYPE_MASK_BUFFER;
	}

	(void) duk_push_dynamic_buffer(ctx, 0);
	js_ctx->h_buf = (duk_hbuffer_dynamic *) duk_get_hbuffer(ctx, -1);
	DUK_ASSERT(js_ctx->h_buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_DYNAMIC(js_ctx->h_buf));

	js_ctx->idx_loop = duk_push_object_internal(ctx);
	DUK_ASSERT(js_ctx->idx_loop >= 0);

	/* [ ... buf loop ] */

	/*
	 *  Process replacer/proplist (2nd argument to JSON.stringify)
	 */

	h = duk_get_hobject(ctx, idx_replacer);
	if (h != NULL) {
		if (DUK_HOBJECT_IS_CALLABLE(h)) {
			js_ctx->h_replacer = h;
		} else if (DUK_HOBJECT_GET_CLASS_NUMBER(h) == DUK_HOBJECT_CLASS_ARRAY) {
			/* Here the specification requires correct array index enumeration
			 * which is a bit tricky for sparse arrays (it is handled by the
			 * enum setup code).  We now enumerate ancestors too, although the
			 * specification is not very clear on whether that is required.
			 */

			int plist_idx = 0;
			int enum_flags;

			js_ctx->idx_proplist = duk_push_array(ctx);  /* FIXME: array internal? */

			enum_flags = DUK_ENUM_ARRAY_INDICES_ONLY |
			             DUK_ENUM_SORT_ARRAY_INDICES;  /* expensive flag */
			duk_enum(ctx, idx_replacer, enum_flags);
			while (duk_next(ctx, -1 /*enum_index*/, 1 /*get_value*/)) {
				/* [ ... proplist enum_obj key val ] */
				if (json_enc_allow_into_proplist(duk_get_tval(ctx, -1))) {
					/* FIXME: duplicates should be eliminated here */
					DUK_DDDPRINT("proplist enum: key=%!T, val=%!T --> accept", duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));
					duk_to_string(ctx, -1);  /* extra coercion of strings is OK */
					duk_put_prop_index(ctx, -4, plist_idx);  /* -> [ ... proplist enum_obj key ] */
					plist_idx++;
					duk_pop(ctx);
				} else {
					DUK_DDDPRINT("proplist enum: key=%!T, val=%!T --> reject", duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));
					duk_pop_2(ctx);
				}
                        }
                        duk_pop(ctx);  /* pop enum */

			/* [ ... proplist ] */
		}
	}

	/* [ ... buf loop (proplist) ] */

	/*
	 *  Process space (3rd argument to JSON.stringify)
	 */

	h = duk_get_hobject(ctx, idx_space);
	if (h != NULL) {
		int c = DUK_HOBJECT_GET_CLASS_NUMBER(h);
		if (c == DUK_HOBJECT_CLASS_NUMBER) {
			duk_to_number(ctx, idx_space);
		} else if (c == DUK_HOBJECT_CLASS_STRING) {
			duk_to_string(ctx, idx_space);
		}
	}

	if (duk_is_number(ctx, idx_space)) {
		double d;
		int nspace;
		char spaces[10] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };  /* FIXME:helper */

		/* ToInteger() coercion; NaN -> 0, infinities are clamped to 0 and 10 */
		/* FIXME: get_clamped_int; double arithmetic is expensive */
		(void) duk_to_int(ctx, idx_space);
		d = duk_get_number(ctx, idx_space);
		if (d > 10) {
			nspace = 10;
		} else if (d < 1) {
			nspace = 0;
		} else {
			nspace = (int) d;
		}
		DUK_ASSERT(nspace >= 0 && nspace <= 10);

		duk_push_lstring(ctx, spaces, nspace);
		js_ctx->h_gap = duk_get_hstring(ctx, -1);
		DUK_ASSERT(js_ctx->h_gap != NULL);
	} else if (duk_is_string(ctx, idx_space)) {
		/* FIXME: substring in-place at idx_place? */
		duk_dup(ctx, idx_space);
		duk_substring(ctx, -1, 0, 10);  /* clamp to 10 chars */
		js_ctx->h_gap = duk_get_hstring(ctx, -1);
		DUK_ASSERT(js_ctx->h_gap != NULL);
	} else {
		/* nop */
	}

	if (js_ctx->h_gap != NULL) {
		/* if gap is empty, behave as if not given at all */
		if (DUK_HSTRING_GET_CHARLEN(js_ctx->h_gap) == 0) {
			js_ctx->h_gap = NULL;
		} else {
			/* set 'indent' only if it will actually increase */
			js_ctx->h_indent = DUK_HTHREAD_STRING_EMPTY_STRING(thr);
		}
	}

	DUK_ASSERT((js_ctx->h_gap == NULL && js_ctx->h_indent == NULL) ||
	           (js_ctx->h_gap != NULL && js_ctx->h_indent != NULL));

	/* [ ... buf loop (proplist) (gap) ] */

	/*
	 *  Create wrapper object and serialize
	 */

	idx_holder = duk_push_object(ctx);
	duk_dup(ctx, idx_value);
	duk_put_prop_stridx(ctx, -2, DUK_STRIDX_EMPTY_STRING);

	DUK_DDDPRINT("before: flags=0x%08x, buf=%!O, loop=%!T, replacer=%!O, proplist=%!T, gap=%!O, indent=%!O, holder=%!T",
	             js_ctx->flags,
	             js_ctx->h_buf,
	             duk_get_tval(ctx, js_ctx->idx_loop),
	             js_ctx->h_replacer,
	             js_ctx->idx_proplist >= 0 ? duk_get_tval(ctx, js_ctx->idx_proplist) : NULL,
	             js_ctx->h_gap,
	             js_ctx->h_indent,
	             duk_get_tval(ctx, -1));
	
	/* serialize the wrapper with empty string key */

	duk_push_hstring_stridx(ctx, DUK_STRIDX_EMPTY_STRING);

	/* [ ... buf loop (proplist) (gap) holder "" ] */

	undef = json_enc_value1(js_ctx, idx_holder);  /* [ ... holder key ] -> [ ... holder key val ] */

	DUK_DDDPRINT("after: flags=0x%08x, buf=%!O, loop=%!T, replacer=%!O, proplist=%!T, gap=%!O, indent=%!O, holder=%!T",
	             js_ctx->flags,
	             js_ctx->h_buf,
	             duk_get_tval(ctx, js_ctx->idx_loop),
	             js_ctx->h_replacer,
	             js_ctx->idx_proplist >= 0 ? duk_get_tval(ctx, js_ctx->idx_proplist) : NULL,
	             js_ctx->h_gap,
	             js_ctx->h_indent,
	             duk_get_tval(ctx, -3));

	if (undef) {
		/*
		 *  Result is undefined
		 */

		duk_push_undefined(ctx);
	} else {
		/*
		 *  Finish and convert buffer to result string
		 */

		json_enc_value2(js_ctx);  /* [ ... key val ] -> [ ... ] */
		DUK_ASSERT(js_ctx->h_buf != NULL);
		duk_push_hbuffer(ctx, (duk_hbuffer *) js_ctx->h_buf);
		duk_to_string(ctx, -1);
	}

	/* The stack has a variable shape here, so force it to the
	 * desired one explicitly.
	 */

	duk_replace(ctx, top_at_entry);
	duk_set_top(ctx, top_at_entry + 1);

	DUK_DDDPRINT("JSON stringify end: value=%!T, replacer=%!T, space=%!T, flags=0x%08x, result=%!T, stack_top=%d",
	             duk_get_tval(ctx, idx_value), duk_get_tval(ctx, idx_replacer),
	             duk_get_tval(ctx, idx_space), flags, duk_get_tval(ctx, -1), duk_get_top(ctx));

	DUK_ASSERT(duk_get_top(ctx) == top_at_entry + 1);
}

/*
 *  Entry points
 */

int duk_builtin_json_object_parse(duk_context *ctx) {
	duk_builtin_json_parse_helper(ctx,
	                              0 /*idx_value*/,
	                              1 /*idx_replacer*/,
	                              0 /*flags*/);
	return 1;
}

int duk_builtin_json_object_stringify(duk_context *ctx) {
	duk_builtin_json_stringify_helper(ctx,
	                                  0 /*idx_value*/,
	                                  1 /*idx_replacer*/,
	                                  2 /*idx_space*/,
	                                  0 /*flags*/);
	return 1;
}

