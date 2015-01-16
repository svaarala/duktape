/*
 *  JSON built-ins.
 *
 *  See doc/json.txt.
 *
 *  Codepoints are handled as duk_uint_fast32_t to ensure that the full
 *  unsigned 32-bit range is supported.  This matters to e.g. JX.
 */

#include "duk_internal.h"

/*
 *  Local defines and forward declarations.
 */

DUK_LOCAL_DECL void duk__dec_syntax_error(duk_json_dec_ctx *js_ctx);
DUK_LOCAL_DECL void duk__dec_eat_white(duk_json_dec_ctx *js_ctx);
DUK_LOCAL_DECL duk_small_int_t duk__dec_peek(duk_json_dec_ctx *js_ctx);
DUK_LOCAL_DECL duk_small_int_t duk__dec_get(duk_json_dec_ctx *js_ctx);
DUK_LOCAL_DECL duk_small_int_t duk__dec_get_nonwhite(duk_json_dec_ctx *js_ctx);
DUK_LOCAL_DECL duk_uint_fast32_t duk__dec_decode_hex_escape(duk_json_dec_ctx *js_ctx, duk_small_uint_t n);
DUK_LOCAL_DECL void duk__dec_req_stridx(duk_json_dec_ctx *js_ctx, duk_small_uint_t stridx);
DUK_LOCAL_DECL void duk__dec_string(duk_json_dec_ctx *js_ctx);
#ifdef DUK_USE_JX
DUK_LOCAL_DECL void duk__dec_plain_string(duk_json_dec_ctx *js_ctx);
DUK_LOCAL_DECL void duk__dec_pointer(duk_json_dec_ctx *js_ctx);
DUK_LOCAL_DECL void duk__dec_buffer(duk_json_dec_ctx *js_ctx);
#endif
DUK_LOCAL_DECL void duk__dec_number(duk_json_dec_ctx *js_ctx);
DUK_LOCAL_DECL void duk__dec_objarr_entry(duk_json_dec_ctx *js_ctx);
DUK_LOCAL_DECL void duk__dec_objarr_exit(duk_json_dec_ctx *js_ctx);
DUK_LOCAL_DECL void duk__dec_object(duk_json_dec_ctx *js_ctx);
DUK_LOCAL_DECL void duk__dec_array(duk_json_dec_ctx *js_ctx);
DUK_LOCAL_DECL void duk__dec_value(duk_json_dec_ctx *js_ctx);
DUK_LOCAL_DECL void duk__dec_reviver_walk(duk_json_dec_ctx *js_ctx);

DUK_LOCAL_DECL void duk__emit_1(duk_json_enc_ctx *js_ctx, duk_uint_fast8_t ch);
DUK_LOCAL_DECL void duk__emit_2(duk_json_enc_ctx *js_ctx, duk_uint_fast16_t packed_chars);
DUK_LOCAL_DECL void duk__emit_esc_auto(duk_json_enc_ctx *js_ctx, duk_uint_fast32_t cp);
DUK_LOCAL_DECL void duk__emit_xutf8(duk_json_enc_ctx *js_ctx, duk_uint_fast32_t cp);
DUK_LOCAL_DECL void duk__emit_hstring(duk_json_enc_ctx *js_ctx, duk_hstring *h);
#if defined(DUK_USE_JX) || defined(DUK_USE_JC)
DUK_LOCAL_DECL void duk__emit_cstring(duk_json_enc_ctx *js_ctx, const char *p);
#endif
DUK_LOCAL_DECL void duk__emit_stridx(duk_json_enc_ctx *js_ctx, duk_small_uint_t stridx);
DUK_LOCAL_DECL duk_bool_t duk__enc_key_quotes_needed(duk_hstring *h_key);
DUK_LOCAL_DECL void duk__enc_quote_string(duk_json_enc_ctx *js_ctx, duk_hstring *h_str);
DUK_LOCAL_DECL void duk__enc_objarr_entry(duk_json_enc_ctx *js_ctx, duk_hstring **h_stepback, duk_hstring **h_indent, duk_idx_t *entry_top);
DUK_LOCAL_DECL void duk__enc_objarr_exit(duk_json_enc_ctx *js_ctx, duk_hstring **h_stepback, duk_hstring **h_indent, duk_idx_t *entry_top);
DUK_LOCAL_DECL void duk__enc_object(duk_json_enc_ctx *js_ctx);
DUK_LOCAL_DECL void duk__enc_array(duk_json_enc_ctx *js_ctx);
DUK_LOCAL_DECL duk_bool_t duk__enc_value1(duk_json_enc_ctx *js_ctx, duk_idx_t idx_holder);
DUK_LOCAL_DECL void duk__enc_value2(duk_json_enc_ctx *js_ctx);
DUK_LOCAL_DECL duk_bool_t duk__enc_allow_into_proplist(duk_tval *tv);

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

DUK_LOCAL void duk__dec_syntax_error(duk_json_dec_ctx *js_ctx) {
	/* Shared handler to minimize parser size.  Cause will be
	 * hidden, unfortunately, but we'll have an offset which
	 * is often quite enough.
	 */
	DUK_ERROR(js_ctx->thr, DUK_ERR_SYNTAX_ERROR, DUK_STR_FMT_INVALID_JSON,
	         (long) (js_ctx->p - js_ctx->p_start));
}

DUK_LOCAL void duk__dec_eat_white(duk_json_dec_ctx *js_ctx) {
	duk_small_uint_t t;
	for (;;) {
		if (js_ctx->p >= js_ctx->p_end) {
			break;
		}
		t = (*js_ctx->p);
		if (!(t == 0x20 || t == 0x0a || t == 0x0d || t == 0x09)) {
			break;
		}
		js_ctx->p++;
	}
}

DUK_LOCAL duk_small_int_t duk__dec_peek(duk_json_dec_ctx *js_ctx) {
	if (js_ctx->p >= js_ctx->p_end) {
		return -1;
	} else {
		return (duk_small_int_t) (*js_ctx->p);
	}
}

DUK_LOCAL duk_small_int_t duk__dec_get(duk_json_dec_ctx *js_ctx) {
	/* Multiple EOFs will now be supplied to the caller.  This could also be
	 * changed so that reading the second EOF would cause an error automatically.
	 */
	if (js_ctx->p >= js_ctx->p_end) {
		return -1;
	} else {
		return (duk_small_int_t) (*js_ctx->p++);
	}
}

DUK_LOCAL duk_small_int_t duk__dec_get_nonwhite(duk_json_dec_ctx *js_ctx) {
	duk__dec_eat_white(js_ctx);
	return duk__dec_get(js_ctx);
}

/* For JX, expressing the whole unsigned 32-bit range matters. */
DUK_LOCAL duk_uint_fast32_t duk__dec_decode_hex_escape(duk_json_dec_ctx *js_ctx, duk_small_uint_t n) {
	duk_small_uint_t i;
	duk_uint_fast32_t res = 0;
	duk_small_int_t x;

	for (i = 0; i < n; i++) {
		/* XXX: share helper from lexer; duk_lexer.c / hexval(). */

		x = duk__dec_get(js_ctx);
		DUK_ASSERT((x >= 0 && x <= 0xff) || (x == -1));

		DUK_DDD(DUK_DDDPRINT("decode_hex_escape: i=%ld, n=%ld, res=%ld, x=%ld",
		                     (long) i, (long) n, (long) res, (long) x));

		/* x == -1 will map to 0xff, dectab returns -1 which causes syntax_error */
		x = duk_hex_dectab[x & 0xff];
		if (DUK_LIKELY(x >= 0)) {
			res = (res * 16) + x;
		} else {
			/* catches EOF and invalid digits */
			goto syntax_error;
		}
	}

	DUK_DDD(DUK_DDDPRINT("final hex decoded value: %ld", (long) res));
	return res;

 syntax_error:
	duk__dec_syntax_error(js_ctx);
	DUK_UNREACHABLE();
	return 0;
}

DUK_LOCAL void duk__dec_req_stridx(duk_json_dec_ctx *js_ctx, duk_small_uint_t stridx) {
	duk_hstring *h;
	duk_uint8_t *p;
	duk_uint8_t *p_end;
	duk_small_int_t x;

	/* First character has already been eaten and checked by the caller. */

	DUK_ASSERT_DISABLE(stridx >= 0);  /* unsigned */
	DUK_ASSERT(stridx < DUK_HEAP_NUM_STRINGS);
	h = DUK_HTHREAD_GET_STRING(js_ctx->thr, stridx);
	DUK_ASSERT(h != NULL);

	p = (duk_uint8_t *) DUK_HSTRING_GET_DATA(h);
	p_end = ((duk_uint8_t *) DUK_HSTRING_GET_DATA(h)) +
	        DUK_HSTRING_GET_BYTELEN(h);

	DUK_ASSERT(*(js_ctx->p - 1) == *p);  /* first character has been matched */
	p++;  /* first char */

	while (p < p_end) {
		x = duk__dec_get(js_ctx);
		if ((duk_small_int_t) (*p) != x) {
			/* catches EOF */
			goto syntax_error;
		}
		p++;
	}

	return;

 syntax_error:
	duk__dec_syntax_error(js_ctx);
	DUK_UNREACHABLE();
}

DUK_LOCAL void duk__dec_string(duk_json_dec_ctx *js_ctx) {
	duk_hthread *thr = js_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	duk_hbuffer_dynamic *h_buf;
	duk_small_int_t x;
	duk_uint_fast32_t cp;

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
		x = duk__dec_get(js_ctx);
		if (x == DUK_ASC_DOUBLEQUOTE) {
			break;
		} else if (x == DUK_ASC_BACKSLASH) {
			/* EOF (-1) will be cast to an unsigned value first
			 * and then re-cast for the switch.  In any case, it
			 * will match the default case (syntax error).
			 */
			cp = (duk_uint_fast32_t) duk__dec_get(js_ctx);
			switch ((int) cp) {
			case DUK_ASC_BACKSLASH: break;
			case DUK_ASC_DOUBLEQUOTE: break;
			case DUK_ASC_SLASH: break;
			case DUK_ASC_LC_T: cp = 0x09; break;
			case DUK_ASC_LC_N: cp = 0x0a; break;
			case DUK_ASC_LC_R: cp = 0x0d; break;
			case DUK_ASC_LC_F: cp = 0x0c; break;
			case DUK_ASC_LC_B: cp = 0x08; break;
			case DUK_ASC_LC_U: {
				cp = duk__dec_decode_hex_escape(js_ctx, 4);
				break;
			}
#ifdef DUK_USE_JX
			case DUK_ASC_UC_U: {
				if (js_ctx->flag_ext_custom) {
					cp = duk__dec_decode_hex_escape(js_ctx, 8);
				} else {
					goto syntax_error;
				}
				break;
			}
			case DUK_ASC_LC_X: {
				if (js_ctx->flag_ext_custom) {
					cp = duk__dec_decode_hex_escape(js_ctx, 2);
				} else {
					goto syntax_error;
				}
				break;
			}
#endif  /* DUK_USE_JX */
			default:
				/* catches EOF (-1) */
				goto syntax_error;
			}
			duk_hbuffer_append_xutf8(thr, h_buf, (duk_uint32_t) cp);
		} else if (x < 0x20) {
			/* catches EOF (-1) */
			goto syntax_error;
		} else {
			duk_hbuffer_append_byte(thr, h_buf, (duk_uint8_t) x);
		}
	}

	duk_to_string(ctx, -1);

	/* [ ... str ] */

	return;

 syntax_error:
	duk__dec_syntax_error(js_ctx);
	DUK_UNREACHABLE();
}

#ifdef DUK_USE_JX
/* Decode a plain string consisting entirely of identifier characters.
 * Used to parse plain keys (e.g. "foo: 123").
 */
DUK_LOCAL void duk__dec_plain_string(duk_json_dec_ctx *js_ctx) {
	duk_hthread *thr = js_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	const duk_uint8_t *p;
	duk_small_int_t x;

	/* Caller has already eaten the first char so backtrack one byte. */

	js_ctx->p--;  /* safe */
	p = js_ctx->p;

	/* Here again we parse bytes, and non-ASCII UTF-8 will cause end of
	 * parsing (which is correct except if there are non-shortest encodings).
	 * There is also no need to check explicitly for end of input buffer as
	 * the input is NUL padded and NUL will exit the parsing loop.
	 *
	 * Because no unescaping takes place, we can just scan to the end of the
	 * plain string and intern from the input buffer.
	 */

	for (;;) {
		x = *p;

		/* There is no need to check the first character specially here
		 * (i.e. reject digits): the caller only accepts valid initial
		 * characters and won't call us if the first character is a digit.
		 * This also ensures that the plain string won't be empty.
		 */

		if (!duk_unicode_is_identifier_part((duk_codepoint_t) x)) {
			break;
		}
		p++;
	}

	duk_push_lstring(ctx, (const char *) js_ctx->p, (duk_size_t) (p - js_ctx->p));
	js_ctx->p = p;

	/* [ ... str ] */
}
#endif  /* DUK_USE_JX */

#ifdef DUK_USE_JX
DUK_LOCAL void duk__dec_pointer(duk_json_dec_ctx *js_ctx) {
	duk_hthread *thr = js_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	const duk_uint8_t *p;
	duk_small_int_t x;
	void *voidptr;

	/* Caller has already eaten the first character ('(') which we don't need. */

	p = js_ctx->p;

	for (;;) {
		x = *p;

		/* Assume that the native representation never contains a closing
		 * parenthesis.
		 */

		if (x == DUK_ASC_RPAREN) {
			break;
		} else if (x <= 0) {
			/* NUL term or -1 (EOF), NUL check would suffice */
			goto syntax_error;
		}
		p++;
	}

	/* There is no need to NUL delimit the sscanf() call: trailing garbage is
	 * ignored and there is always a NUL terminator which will force an error
	 * if no error is encountered before it.  It's possible that the scan
	 * would scan further than between [js_ctx->p,p[ though and we'd advance
	 * by less than the scanned value.
	 *
	 * Because pointers are platform specific, a failure to scan a pointer
	 * results in a null pointer which is a better placeholder than a missing
	 * value or an error.
	 */

	voidptr = NULL;
	(void) DUK_SSCANF((const char *) js_ctx->p, DUK_STR_FMT_PTR, &voidptr);
	duk_push_pointer(ctx, voidptr);
	js_ctx->p = p + 1;  /* skip ')' */

	/* [ ... ptr ] */

	return;

 syntax_error:
	duk__dec_syntax_error(js_ctx);
	DUK_UNREACHABLE();
}
#endif  /* DUK_USE_JX */

#ifdef DUK_USE_JX
DUK_LOCAL void duk__dec_buffer(duk_json_dec_ctx *js_ctx) {
	duk_hthread *thr = js_ctx->thr;
	duk_context *ctx = (duk_context *) thr;
	const duk_uint8_t *p;
	duk_small_int_t x;

	/* Caller has already eaten the first character ('|') which we don't need. */

	p = js_ctx->p;

	for (;;) {
		x = *p;

		/* This loop intentionally does not ensure characters are valid
		 * ([0-9a-fA-F]) because the hex decode call below will do that.
		 */
		if (x == DUK_ASC_PIPE) {
			break;
		} else if (x <= 0) {
			/* NUL term or -1 (EOF), NUL check would suffice */
			goto syntax_error;
		}
		p++;
	}

	duk_push_lstring(ctx, (const char *) js_ctx->p, (duk_size_t) (p - js_ctx->p));
	duk_hex_decode(ctx, -1);
	js_ctx->p = p + 1;  /* skip '|' */

	/* [ ... buf ] */

	return;

 syntax_error:
	duk__dec_syntax_error(js_ctx);
	DUK_UNREACHABLE();
}
#endif  /* DUK_USE_JX */

/* Parse a number, other than NaN or +/- Infinity */
DUK_LOCAL void duk__dec_number(duk_json_dec_ctx *js_ctx) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	const duk_uint8_t *p_start;
	duk_small_int_t x;
	duk_small_uint_t s2n_flags;

	DUK_DDD(DUK_DDDPRINT("parse_number"));

	/* Caller has already eaten the first character so backtrack one
	 * byte.  This is correct because the first character is either
	 * '-' or a digit (i.e. an ASCII character).
	 */

	js_ctx->p--;  /* safe */
	p_start = js_ctx->p;

	/* First pass parse is very lenient (e.g. allows '1.2.3') and extracts a
	 * string for strict number parsing.
	 */

	for (;;) {
		x = duk__dec_peek(js_ctx);

		DUK_DDD(DUK_DDDPRINT("parse_number: p_start=%p, p=%p, p_end=%p, x=%ld",
		                     (void *) p_start, (void *) js_ctx->p,
		                     (void *) js_ctx->p_end, (long) x));

		if (!((x >= DUK_ASC_0 && x <= DUK_ASC_9) ||
		      (x == DUK_ASC_PERIOD || x == DUK_ASC_LC_E ||
		       x == DUK_ASC_UC_E || x == DUK_ASC_MINUS || x == DUK_ASC_PLUS))) {
			/* Plus sign must be accepted for positive exponents
			 * (e.g. '1.5e+2').
			 */
			break;
		}

		js_ctx->p++;  /* safe, because matched char */
	}

	DUK_ASSERT(js_ctx->p > p_start);
	duk_push_lstring(ctx, (const char *) p_start, (duk_size_t) (js_ctx->p - p_start));

	s2n_flags = DUK_S2N_FLAG_ALLOW_EXP |
	            DUK_S2N_FLAG_ALLOW_MINUS |  /* but don't allow leading plus */
	            DUK_S2N_FLAG_ALLOW_FRAC;

	DUK_DDD(DUK_DDDPRINT("parse_number: string before parsing: %!T",
	                     (duk_tval *) duk_get_tval(ctx, -1)));
	duk_numconv_parse(ctx, 10 /*radix*/, s2n_flags);
	if (duk_is_nan(ctx, -1)) {
		duk__dec_syntax_error(js_ctx);
	}
	DUK_ASSERT(duk_is_number(ctx, -1));
	DUK_DDD(DUK_DDDPRINT("parse_number: final number: %!T",
	                     (duk_tval *) duk_get_tval(ctx, -1)));

	/* [ ... num ] */
}

DUK_LOCAL void duk__dec_objarr_entry(duk_json_dec_ctx *js_ctx) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_require_stack(ctx, DUK_JSON_DEC_REQSTACK);

	/* c recursion check */

	DUK_ASSERT(js_ctx->recursion_depth >= 0);
	DUK_ASSERT(js_ctx->recursion_depth <= js_ctx->recursion_limit);
	if (js_ctx->recursion_depth >= js_ctx->recursion_limit) {
		DUK_ERROR((duk_hthread *) ctx, DUK_ERR_RANGE_ERROR, DUK_STR_JSONDEC_RECLIMIT);
	}
	js_ctx->recursion_depth++;
}

DUK_LOCAL void duk__dec_objarr_exit(duk_json_dec_ctx *js_ctx) {
	/* c recursion check */

	DUK_ASSERT(js_ctx->recursion_depth > 0);
	DUK_ASSERT(js_ctx->recursion_depth <= js_ctx->recursion_limit);
	js_ctx->recursion_depth--;
}

DUK_LOCAL void duk__dec_object(duk_json_dec_ctx *js_ctx) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_int_t key_count;  /* XXX: a "first" flag would suffice */
	duk_small_int_t x;

	DUK_DDD(DUK_DDDPRINT("parse_object"));

	duk__dec_objarr_entry(js_ctx);

	duk_push_object(ctx);

	/* Initial '{' has been checked and eaten by caller. */

	key_count = 0;
	for (;;) {
		x = duk__dec_get_nonwhite(js_ctx);

		DUK_DDD(DUK_DDDPRINT("parse_object: obj=%!T, x=%ld, key_count=%ld",
		                     (duk_tval *) duk_get_tval(ctx, -1),
		                     (long) x, (long) key_count));

		/* handle comma and closing brace */

		if (x == DUK_ASC_COMMA && key_count > 0) {
			/* accept comma, expect new value */
			x = duk__dec_get_nonwhite(js_ctx);
		} else if (x == DUK_ASC_RCURLY) {
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

		if (x == DUK_ASC_DOUBLEQUOTE) {
			duk__dec_string(js_ctx);
#ifdef DUK_USE_JX
		} else if (js_ctx->flag_ext_custom &&
		           duk_unicode_is_identifier_start((duk_codepoint_t) x)) {
			duk__dec_plain_string(js_ctx);
#endif
		} else {
			goto syntax_error;
		}

		/* [ ... obj key ] */

		x = duk__dec_get_nonwhite(js_ctx);
		if (x != DUK_ASC_COLON) {
			goto syntax_error;
		}

		duk__dec_value(js_ctx);

		/* [ ... obj key val ] */

		duk_xdef_prop_wec(ctx, -3);

		/* [ ... obj ] */

		key_count++;
	}

	/* [ ... obj ] */

	DUK_DDD(DUK_DDDPRINT("parse_object: final object is %!T",
	                     (duk_tval *) duk_get_tval(ctx, -1)));

	duk__dec_objarr_exit(js_ctx);
	return;

 syntax_error:
	duk__dec_syntax_error(js_ctx);
	DUK_UNREACHABLE();
}

DUK_LOCAL void duk__dec_array(duk_json_dec_ctx *js_ctx) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_uarridx_t arr_idx;
	duk_small_int_t x;

	DUK_DDD(DUK_DDDPRINT("parse_array"));

	duk__dec_objarr_entry(js_ctx);

	duk_push_array(ctx);

	/* Initial '[' has been checked and eaten by caller. */

	arr_idx = 0;
	for (;;) {
		x = duk__dec_get_nonwhite(js_ctx);

		DUK_DDD(DUK_DDDPRINT("parse_array: arr=%!T, x=%ld, arr_idx=%ld",
		                     (duk_tval *) duk_get_tval(ctx, -1),
		                     (long) x, (long) arr_idx));

		/* handle comma and closing bracket */

		if ((x == DUK_ASC_COMMA) && (arr_idx != 0)) {
			/* accept comma, expect new value */
			;
		} else if (x == DUK_ASC_RBRACKET) {
			/* eat closing bracket */
			break;
		} else if (arr_idx == 0) {
			/* accept anything, expect first value (EOF will be
			 * caught by duk__dec_value() below.
			 */
			js_ctx->p--;  /* backtrack (safe) */
		} else {
			/* catches EOF (and initial comma) */
			goto syntax_error;
		}

		/* parse value */

		duk__dec_value(js_ctx);

		/* [ ... arr val ] */

		duk_xdef_prop_index_wec(ctx, -2, arr_idx);
		arr_idx++;
	}

	/* Must set 'length' explicitly when using duk_xdef_prop_xxx() to
	 * set the values.
	 */

	duk_set_length(ctx, -1, arr_idx);

	/* [ ... arr ] */

	DUK_DDD(DUK_DDDPRINT("parse_array: final array is %!T",
	                     (duk_tval *) duk_get_tval(ctx, -1)));

	duk__dec_objarr_exit(js_ctx);
	return;

 syntax_error:
	duk__dec_syntax_error(js_ctx);
	DUK_UNREACHABLE();
}

DUK_LOCAL void duk__dec_value(duk_json_dec_ctx *js_ctx) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_small_int_t x;

	x = duk__dec_get_nonwhite(js_ctx);

	DUK_DDD(DUK_DDDPRINT("parse_value: initial x=%ld", (long) x));

	/* Note: duk__dec_req_stridx() backtracks one char */

	if (x == DUK_ASC_DOUBLEQUOTE) {
		duk__dec_string(js_ctx);
	} else if ((x >= DUK_ASC_0 && x <= DUK_ASC_9) || (x == DUK_ASC_MINUS)) {
#ifdef DUK_USE_JX
		if (js_ctx->flag_ext_custom && duk__dec_peek(js_ctx) == DUK_ASC_UC_I) {
			duk__dec_req_stridx(js_ctx, DUK_STRIDX_MINUS_INFINITY);  /* "-Infinity" */
			duk_push_number(ctx, -DUK_DOUBLE_INFINITY);
		} else {
#else
		{  /* unconditional block */
#endif
			/* We already ate 'x', so duk__dec_number() will back up one byte. */
			duk__dec_number(js_ctx);
		}
	} else if (x == DUK_ASC_LC_T) {
		duk__dec_req_stridx(js_ctx, DUK_STRIDX_TRUE);
		duk_push_true(ctx);
	} else if (x == DUK_ASC_LC_F) {
		duk__dec_req_stridx(js_ctx, DUK_STRIDX_FALSE);
		duk_push_false(ctx);
	} else if (x == DUK_ASC_LC_N) {
		duk__dec_req_stridx(js_ctx, DUK_STRIDX_LC_NULL);
		duk_push_null(ctx);
#ifdef DUK_USE_JX
	} else if (js_ctx->flag_ext_custom && x == DUK_ASC_LC_U) {
		duk__dec_req_stridx(js_ctx, DUK_STRIDX_LC_UNDEFINED);
		duk_push_undefined(ctx);
	} else if (js_ctx->flag_ext_custom && x == DUK_ASC_UC_N) {
		duk__dec_req_stridx(js_ctx, DUK_STRIDX_NAN);
		duk_push_nan(ctx);
	} else if (js_ctx->flag_ext_custom && x == DUK_ASC_UC_I) {
		duk__dec_req_stridx(js_ctx, DUK_STRIDX_INFINITY);
		duk_push_number(ctx, DUK_DOUBLE_INFINITY);
	} else if (js_ctx->flag_ext_custom && x == DUK_ASC_LPAREN) {
		duk__dec_pointer(js_ctx);
	} else if (js_ctx->flag_ext_custom && x == DUK_ASC_PIPE) {
		duk__dec_buffer(js_ctx);
#endif
	} else if (x == DUK_ASC_LCURLY) {
		duk__dec_object(js_ctx);
	} else if (x == DUK_ASC_LBRACKET) {
		duk__dec_array(js_ctx);
	} else {
		/* catches EOF */
		goto syntax_error;
	}

	duk__dec_eat_white(js_ctx);

	/* [ ... val ] */
	return;

 syntax_error:
	duk__dec_syntax_error(js_ctx);
	DUK_UNREACHABLE();
}

/* Recursive value reviver, implements the Walk() algorithm.  No C recursion
 * check is done here because the initial parsing step will already ensure
 * there is a reasonable limit on C recursion depth and hence object depth.
 */
DUK_LOCAL void duk__dec_reviver_walk(duk_json_dec_ctx *js_ctx) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_hobject *h;
	duk_uarridx_t i, arr_len;

	DUK_DDD(DUK_DDDPRINT("walk: top=%ld, holder=%!T, name=%!T",
	                     (long) duk_get_top(ctx),
	                     (duk_tval *) duk_get_tval(ctx, -2),
	                     (duk_tval *) duk_get_tval(ctx, -1)));

	duk_dup_top(ctx);
	duk_get_prop(ctx, -3);  /* -> [ ... holder name val ] */

	h = duk_get_hobject(ctx, -1);
	if (h != NULL) {
		if (DUK_HOBJECT_GET_CLASS_NUMBER(h) == DUK_HOBJECT_CLASS_ARRAY) {
			arr_len = (duk_uarridx_t) duk_get_length(ctx, -1);
			for (i = 0; i < arr_len; i++) {
				/* [ ... holder name val ] */

				DUK_DDD(DUK_DDDPRINT("walk: array, top=%ld, i=%ld, arr_len=%ld, holder=%!T, name=%!T, val=%!T",
				                     (long) duk_get_top(ctx), (long) i, (long) arr_len,
				                     (duk_tval *) duk_get_tval(ctx, -3), (duk_tval *) duk_get_tval(ctx, -2),
				                     (duk_tval *) duk_get_tval(ctx, -1)));

				/* XXX: push_uint_string / push_u32_string */
				duk_dup_top(ctx);
				duk_push_uint(ctx, (duk_uint_t) i);
				duk_to_string(ctx, -1);  /* -> [ ... holder name val val ToString(i) ] */
				duk__dec_reviver_walk(js_ctx);  /* -> [ ... holder name val new_elem ] */

				if (duk_is_undefined(ctx, -1)) {
					duk_pop(ctx);
					duk_del_prop_index(ctx, -1, i);
				} else {
					/* XXX: duk_xdef_prop_index_wec() would be more appropriate
					 * here but it currently makes some assumptions that might
					 * not hold (e.g. that previous property is not an accessor).
					 */
					duk_put_prop_index(ctx, -2, i);
				}
			}
		} else {
			/* [ ... holder name val ] */
			duk_enum(ctx, -1, DUK_ENUM_OWN_PROPERTIES_ONLY /*flags*/);
			while (duk_next(ctx, -1 /*enum_index*/, 0 /*get_value*/)) {
				DUK_DDD(DUK_DDDPRINT("walk: object, top=%ld, holder=%!T, name=%!T, val=%!T, enum=%!iT, obj_key=%!T",
				                     (long) duk_get_top(ctx), (duk_tval *) duk_get_tval(ctx, -5),
				                     (duk_tval *) duk_get_tval(ctx, -4), (duk_tval *) duk_get_tval(ctx, -3),
				                     (duk_tval *) duk_get_tval(ctx, -2), (duk_tval *) duk_get_tval(ctx, -1)));

				/* [ ... holder name val enum obj_key ] */
				duk_dup(ctx, -3);
				duk_dup(ctx, -2);

				/* [ ... holder name val enum obj_key val obj_key ] */
				duk__dec_reviver_walk(js_ctx);

				/* [ ... holder name val enum obj_key new_elem ] */
				if (duk_is_undefined(ctx, -1)) {
					duk_pop(ctx);
					duk_del_prop(ctx, -3);
				} else {
					/* XXX: duk_xdef_prop_index_wec() would be more appropriate
					 * here but it currently makes some assumptions that might
					 * not hold (e.g. that previous property is not an accessor).
					 *
					 * Using duk_put_prop() works incorrectly with '__proto__'
					 * if the own property with that name has been deleted.  This
					 * does not happen normally, but a clever reviver can trigger
					 * that, see complex reviver case in: test-bug-json-parse-__proto__.js.
					 */
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

	DUK_DDD(DUK_DDDPRINT("walk: top=%ld, result=%!T",
	                     (long) duk_get_top(ctx), (duk_tval *) duk_get_tval(ctx, -1)));
}

/*
 *  Stringify implementation.
 */

#define DUK__EMIT_1(js_ctx,ch)          duk__emit_1((js_ctx), (duk_uint_fast8_t) (ch))
#define DUK__EMIT_2(js_ctx,ch1,ch2)     duk__emit_2((js_ctx), (((duk_uint_fast16_t)(ch1)) << 8) + (duk_uint_fast16_t)(ch2))
#define DUK__EMIT_ESC_AUTO(js_ctx,cp)   duk__emit_esc_auto((js_ctx), (cp))
#define DUK__EMIT_XUTF8(js_ctx,cp)      duk__emit_xutf8((js_ctx), (cp))
#define DUK__EMIT_HSTR(js_ctx,h)        duk__emit_hstring((js_ctx), (h))
#if defined(DUK_USE_JX) || defined(DUK_USE_JC)
#define DUK__EMIT_CSTR(js_ctx,p)        duk__emit_cstring((js_ctx), (p))
#endif
#define DUK__EMIT_STRIDX(js_ctx,i)      duk__emit_stridx((js_ctx), (i))

DUK_LOCAL void duk__emit_1(duk_json_enc_ctx *js_ctx, duk_uint_fast8_t ch) {
	duk_hbuffer_append_byte(js_ctx->thr, js_ctx->h_buf, (duk_uint8_t) ch);
}

DUK_LOCAL void duk__emit_2(duk_json_enc_ctx *js_ctx, duk_uint_fast16_t packed_chars) {
	duk_uint8_t buf[2];
	buf[0] = (duk_uint8_t) (packed_chars >> 8);
	buf[1] = (duk_uint8_t) (packed_chars & 0xff);
	duk_hbuffer_append_bytes(js_ctx->thr, js_ctx->h_buf, (duk_uint8_t *) buf, 2);
}

#define DUK__MKESC(nybbles,esc1,esc2)  \
	(((duk_uint_fast32_t) (nybbles)) << 16) | \
	(((duk_uint_fast32_t) (esc1)) << 8) | \
	((duk_uint_fast32_t) (esc2))

DUK_LOCAL void duk__emit_esc_auto(duk_json_enc_ctx *js_ctx, duk_uint_fast32_t cp) {
	duk_uint8_t buf[2];
	duk_uint_fast32_t tmp;
	duk_small_uint_t dig;

	/* Select appropriate escape format automatically, and set 'tmp' to a
	 * value encoding both the escape format character and the nybble count:
	 *
	 *   (nybble_count << 16) | (escape_char1) | (escape_char2)
	 */

#ifdef DUK_USE_JX
	if (DUK_LIKELY(cp < 0x100UL)) {
		if (DUK_UNLIKELY(js_ctx->flag_ext_custom)) {
			tmp = DUK__MKESC(2, DUK_ASC_BACKSLASH, DUK_ASC_LC_X);
		} else {
			tmp = DUK__MKESC(4, DUK_ASC_BACKSLASH, DUK_ASC_LC_U);
		}
	} else
#endif
	if (DUK_LIKELY(cp < 0x10000UL)) {
		tmp = DUK__MKESC(4, DUK_ASC_BACKSLASH, DUK_ASC_LC_U);
	} else {
#ifdef DUK_USE_JX
		if (DUK_LIKELY(js_ctx->flag_ext_custom)) {
			tmp = DUK__MKESC(8, DUK_ASC_BACKSLASH, DUK_ASC_UC_U);
		} else
#endif
		{
			/* In compatible mode and standard JSON mode, output
			 * something useful for non-BMP characters.  This won't
			 * roundtrip but will still be more or less readable and
			 * more useful than an error.
			 */
			tmp = DUK__MKESC(8, DUK_ASC_UC_U, DUK_ASC_PLUS);
		}
	}

	buf[0] = (duk_uint8_t) ((tmp >> 8) & 0xff);
	buf[1] = (duk_uint8_t) (tmp & 0xff);
	duk_hbuffer_append_bytes(js_ctx->thr, js_ctx->h_buf, buf, 2);

	tmp = tmp >> 16;
	while (tmp > 0) {
		tmp--;
		dig = (duk_small_uint_t) ((cp >> (4 * tmp)) & 0x0f);
		duk_hbuffer_append_byte(js_ctx->thr, js_ctx->h_buf, duk_lc_digits[dig]);
	}
}

DUK_LOCAL void duk__emit_xutf8(duk_json_enc_ctx *js_ctx, duk_uint_fast32_t cp) {
	(void) duk_hbuffer_append_xutf8(js_ctx->thr, js_ctx->h_buf, cp);
}

DUK_LOCAL void duk__emit_hstring(duk_json_enc_ctx *js_ctx, duk_hstring *h) {
	DUK_ASSERT(h != NULL);
	duk_hbuffer_append_bytes(js_ctx->thr,
	                         js_ctx->h_buf,
	                         (duk_uint8_t *) DUK_HSTRING_GET_DATA(h),
	                         (duk_size_t) DUK_HSTRING_GET_BYTELEN(h));
}

#if defined(DUK_USE_JX) || defined(DUK_USE_JC)
DUK_LOCAL void duk__emit_cstring(duk_json_enc_ctx *js_ctx, const char *p) {
	DUK_ASSERT(p != NULL);
	(void) duk_hbuffer_append_cstring(js_ctx->thr, js_ctx->h_buf, p);
}
#endif

DUK_LOCAL void duk__emit_stridx(duk_json_enc_ctx *js_ctx, duk_small_uint_t stridx) {
	DUK_ASSERT_DISABLE(stridx >= 0);  /* unsigned */
	DUK_ASSERT(stridx < DUK_HEAP_NUM_STRINGS);
	duk__emit_hstring(js_ctx, DUK_HTHREAD_GET_STRING(js_ctx->thr, stridx));
}

/* Check whether key quotes would be needed (custom encoding). */
DUK_LOCAL duk_bool_t duk__enc_key_quotes_needed(duk_hstring *h_key) {
	const duk_uint8_t *p, *p_start, *p_end;
	duk_small_uint_t ch;

	DUK_ASSERT(h_key != NULL);
	p_start = DUK_HSTRING_GET_DATA(h_key);
	p_end = p_start + DUK_HSTRING_GET_BYTELEN(h_key);
	p = p_start;

	DUK_DDD(DUK_DDDPRINT("duk__enc_key_quotes_needed: h_key=%!O, p_start=%p, p_end=%p, p=%p",
	                     (duk_heaphdr *) h_key, (void *) p_start, (void *) p_end, (void *) p));

	/* Since we only accept ASCII characters, there is no need for
	 * actual decoding.  A non-ASCII character will be >= 0x80 which
	 * causes a false return value immediately.
	 */

	if (p == p_end) {
		/* Zero length string is not accepted without quotes */
		return 1;
	}

	while (p < p_end) {
		ch = (duk_small_uint_t) (*p);

		/* Accept ASCII IdentifierStart and IdentifierPart if not first char.
		 * Function selection is a bit uncommon.
		 */
		if ((p > p_start ? duk_unicode_is_identifier_part :
		                   duk_unicode_is_identifier_start) ((duk_codepoint_t) ch)) {
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

DUK_LOCAL duk_uint8_t duk__quote_esc[14] = {
	DUK_ASC_NUL, DUK_ASC_NUL, DUK_ASC_NUL, DUK_ASC_NUL,
	DUK_ASC_NUL, DUK_ASC_NUL, DUK_ASC_NUL, DUK_ASC_NUL,
	DUK_ASC_LC_B, DUK_ASC_LC_T, DUK_ASC_LC_N, DUK_ASC_NUL,
	DUK_ASC_LC_F, DUK_ASC_LC_R
};

DUK_LOCAL void duk__enc_quote_string(duk_json_enc_ctx *js_ctx, duk_hstring *h_str) {
	duk_hthread *thr = js_ctx->thr;
	const duk_uint8_t *p, *p_start, *p_end, *p_tmp;
	duk_ucodepoint_t cp;  /* typed for duk_unicode_decode_xutf8() */

	DUK_DDD(DUK_DDDPRINT("duk__enc_quote_string: h_str=%!O", (duk_heaphdr *) h_str));

	DUK_ASSERT(h_str != NULL);
	p_start = DUK_HSTRING_GET_DATA(h_str);
	p_end = p_start + DUK_HSTRING_GET_BYTELEN(h_str);
	p = p_start;

	DUK__EMIT_1(js_ctx, DUK_ASC_DOUBLEQUOTE);

	while (p < p_end) {
		cp = *p;

		if (DUK_LIKELY(cp <= 0x7f)) {
			/* ascii fast path: avoid decoding utf-8 */
			p++;
			if (cp == 0x22 || cp == 0x5c) {
				/* double quote or backslash */
				DUK__EMIT_2(js_ctx, DUK_ASC_BACKSLASH, cp);
			} else if (cp < 0x20) {
				duk_uint_fast8_t esc_char;

				/* This approach is a bit shorter than a straight
				 * if-else-ladder and also a bit faster.
				 */
				if (cp < (sizeof(duk__quote_esc) / sizeof(duk_uint8_t)) &&
				    (esc_char = duk__quote_esc[cp]) != 0) {
					DUK__EMIT_2(js_ctx, DUK_ASC_BACKSLASH, esc_char);
				} else {
					DUK__EMIT_ESC_AUTO(js_ctx, cp);
				}
			} else if (cp == 0x7f && js_ctx->flag_ascii_only) {
				DUK__EMIT_ESC_AUTO(js_ctx, cp);
			} else {
				/* any other printable -> as is */
				DUK__EMIT_1(js_ctx, cp);
			}
		} else {
			/* slow path decode */

			/* If XUTF-8 decoding fails, treat the offending byte as a codepoint directly
			 * and go forward one byte.  This is of course very lossy, but allows some kind
			 * of output to be produced even for internal strings which don't conform to
			 * XUTF-8.  All standard Ecmascript strings are always CESU-8, so this behavior
			 * does not violate the Ecmascript specification.  The behavior is applied to
			 * all modes, including Ecmascript standard JSON.  Because the current XUTF-8
			 * decoding is not very strict, this behavior only really affects initial bytes
			 * and truncated codepoints.
			 *
			 * XXX: another alternative would be to scan forwards to start of next codepoint
			 * (or end of input) and emit just one replacement codepoint.
			 */

			p_tmp = p;
			if (!duk_unicode_decode_xutf8(thr, &p, p_start, p_end, &cp)) {
				/* Decode failed. */
				cp = *p_tmp;
				p = p_tmp + 1;
			}

#ifdef DUK_USE_NONSTD_JSON_ESC_U2028_U2029
			if (js_ctx->flag_ascii_only || cp == 0x2028 || cp == 0x2029) {
#else
			if (js_ctx->flag_ascii_only) {
#endif
				DUK__EMIT_ESC_AUTO(js_ctx, cp);
			} else {
				/* as is */
				DUK__EMIT_XUTF8(js_ctx, cp);
			}
		}
	}

	DUK__EMIT_1(js_ctx, DUK_ASC_DOUBLEQUOTE);
}

/* Shared entry handling for object/array serialization: indent/stepback,
 * loop detection.
 */
DUK_LOCAL void duk__enc_objarr_entry(duk_json_enc_ctx *js_ctx, duk_hstring **h_stepback, duk_hstring **h_indent, duk_idx_t *entry_top) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_hobject *h_target;

	*entry_top = duk_get_top(ctx);

	duk_require_stack(ctx, DUK_JSON_ENC_REQSTACK);

	/* loop check */

	h_target = duk_get_hobject(ctx, -1);  /* object or array */
	DUK_ASSERT(h_target != NULL);
	duk_push_sprintf(ctx, DUK_STR_FMT_PTR, (void *) h_target);

	duk_dup_top(ctx);  /* -> [ ... voidp voidp ] */
	if (duk_has_prop(ctx, js_ctx->idx_loop)) {
		DUK_ERROR((duk_hthread *) ctx, DUK_ERR_TYPE_ERROR, DUK_STR_CYCLIC_INPUT);
	}
	duk_push_true(ctx);  /* -> [ ... voidp true ] */
	duk_put_prop(ctx, js_ctx->idx_loop);  /* -> [ ... ] */

	/* c recursion check */

	DUK_ASSERT(js_ctx->recursion_depth >= 0);
	DUK_ASSERT(js_ctx->recursion_depth <= js_ctx->recursion_limit);
	if (js_ctx->recursion_depth >= js_ctx->recursion_limit) {
		DUK_ERROR((duk_hthread *) ctx, DUK_ERR_RANGE_ERROR, DUK_STR_JSONENC_RECLIMIT);
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

	DUK_DDD(DUK_DDDPRINT("shared entry finished: top=%ld, loop=%!T",
	                     (long) duk_get_top(ctx), (duk_tval *) duk_get_tval(ctx, js_ctx->idx_loop)));
}

/* Shared exit handling for object/array serialization. */
DUK_LOCAL void duk__enc_objarr_exit(duk_json_enc_ctx *js_ctx, duk_hstring **h_stepback, duk_hstring **h_indent, duk_idx_t *entry_top) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_hobject *h_target;

	DUK_UNREF(h_indent);

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
	duk_push_sprintf(ctx, DUK_STR_FMT_PTR, (void *) h_target);

	duk_del_prop(ctx, js_ctx->idx_loop);  /* -> [ ... ] */

	/* restore stack top after unbalanced code paths */
	duk_set_top(ctx, *entry_top);

	DUK_DDD(DUK_DDDPRINT("shared entry finished: top=%ld, loop=%!T",
	                     (long) duk_get_top(ctx), (duk_tval *) duk_get_tval(ctx, js_ctx->idx_loop)));
}

/* The JO(value) operation: encode object.
 *
 * Stack policy: [ object ] -> [ object ].
 */
DUK_LOCAL void duk__enc_object(duk_json_enc_ctx *js_ctx) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_hstring *h_stepback;
	duk_hstring *h_indent;
	duk_hstring *h_key;
	duk_idx_t entry_top;
	duk_idx_t idx_obj;
	duk_idx_t idx_keys;
	duk_bool_t first;
	duk_bool_t undef;
	duk_uarridx_t arr_len, i;

	DUK_DDD(DUK_DDDPRINT("duk__enc_object: obj=%!T", (duk_tval *) duk_get_tval(ctx, -1)));

	duk__enc_objarr_entry(js_ctx, &h_stepback, &h_indent, &entry_top);

	idx_obj = entry_top - 1;

	if (js_ctx->idx_proplist >= 0) {
		idx_keys = js_ctx->idx_proplist;
	} else {
		/* XXX: would be nice to enumerate an object at specified index */
		duk_dup(ctx, idx_obj);
		(void) duk_hobject_get_enumerated_keys(ctx, DUK_ENUM_OWN_PROPERTIES_ONLY /*flags*/);  /* [ ... target ] -> [ ... target keys ] */
		idx_keys = duk_require_normalize_index(ctx, -1);
		/* leave stack unbalanced on purpose */
	}

	DUK_DDD(DUK_DDDPRINT("idx_keys=%ld, h_keys=%!T",
	                     (long) idx_keys, (duk_tval *) duk_get_tval(ctx, idx_keys)));

	/* Steps 8-10 have been merged to avoid a "partial" variable. */

	DUK__EMIT_1(js_ctx, DUK_ASC_LCURLY);

	/* XXX: keys is an internal object with all keys to be processed
	 * in its (gapless) array part.  Because nobody can touch the keys
	 * object, we could iterate its array part directly (keeping in mind
	 * that it can be reallocated).
	 */

	arr_len = (duk_uarridx_t) duk_get_length(ctx, idx_keys);
	first = 1;
	for (i = 0; i < arr_len; i++) {
		duk_get_prop_index(ctx, idx_keys, i);  /* -> [ ... key ] */

		DUK_DDD(DUK_DDDPRINT("object property loop: holder=%!T, key=%!T",
		                     (duk_tval *) duk_get_tval(ctx, idx_obj),
		                     (duk_tval *) duk_get_tval(ctx, -1)));

		undef = duk__enc_value1(js_ctx, idx_obj);
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
			DUK__EMIT_1(js_ctx, DUK_ASC_COMMA);
		}
		if (h_indent != NULL) {
			DUK__EMIT_1(js_ctx, 0x0a);
			DUK__EMIT_HSTR(js_ctx, h_indent);
		}

		h_key = duk_get_hstring(ctx, -2);
		DUK_ASSERT(h_key != NULL);
		if (js_ctx->flag_avoid_key_quotes && !duk__enc_key_quotes_needed(h_key)) {
			/* emit key as is */
			DUK__EMIT_HSTR(js_ctx, h_key);
		} else {
			duk__enc_quote_string(js_ctx, h_key);
		}

		if (h_indent != NULL) {
			DUK__EMIT_2(js_ctx, DUK_ASC_COLON, DUK_ASC_SPACE);
		} else {
			DUK__EMIT_1(js_ctx, DUK_ASC_COLON);
		}

		/* [ ... key val ] */

		duk__enc_value2(js_ctx);  /* -> [ ... ] */
	}

	if (!first) {
		if (h_stepback != NULL) {
			DUK_ASSERT(h_indent != NULL);
			DUK__EMIT_1(js_ctx, 0x0a);
			DUK__EMIT_HSTR(js_ctx, h_stepback);
		}
	}
	DUK__EMIT_1(js_ctx, DUK_ASC_RCURLY);

	duk__enc_objarr_exit(js_ctx, &h_stepback, &h_indent, &entry_top);

	DUK_ASSERT_TOP(ctx, entry_top);
}

/* The JA(value) operation: encode array.
 *
 * Stack policy: [ array ] -> [ array ].
 */
DUK_LOCAL void duk__enc_array(duk_json_enc_ctx *js_ctx) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_hstring *h_stepback;
	duk_hstring *h_indent;
	duk_idx_t entry_top;
	duk_idx_t idx_arr;
	duk_bool_t undef;
	duk_uarridx_t i, arr_len;

	DUK_DDD(DUK_DDDPRINT("duk__enc_array: array=%!T",
	                     (duk_tval *) duk_get_tval(ctx, -1)));

	duk__enc_objarr_entry(js_ctx, &h_stepback, &h_indent, &entry_top);

	idx_arr = entry_top - 1;

	/* Steps 8-10 have been merged to avoid a "partial" variable. */

	DUK__EMIT_1(js_ctx, DUK_ASC_LBRACKET);

	arr_len = (duk_uarridx_t) duk_get_length(ctx, idx_arr);
	for (i = 0; i < arr_len; i++) {
		DUK_DDD(DUK_DDDPRINT("array entry loop: array=%!T, h_indent=%!O, h_stepback=%!O, index=%ld, arr_len=%ld",
		                     (duk_tval *) duk_get_tval(ctx, idx_arr), (duk_heaphdr *) h_indent,
		                     (duk_heaphdr *) h_stepback, (long) i, (long) arr_len));

		if (i > 0) {
			DUK__EMIT_1(js_ctx, DUK_ASC_COMMA);
		}
		if (h_indent != NULL) {
			DUK__EMIT_1(js_ctx, 0x0a);
			DUK__EMIT_HSTR(js_ctx, h_indent);
		}

		/* XXX: duk_push_uint_string() */
		duk_push_uint(ctx, (duk_uint_t) i);
		duk_to_string(ctx, -1);  /* -> [ ... key ] */
		undef = duk__enc_value1(js_ctx, idx_arr);

		if (undef) {
			DUK__EMIT_STRIDX(js_ctx, DUK_STRIDX_LC_NULL);
		} else {
			/* [ ... key val ] */
			duk__enc_value2(js_ctx);
		}
	}

	if (arr_len > 0) {
		if (h_stepback != NULL) {
			DUK_ASSERT(h_indent != NULL);
			DUK__EMIT_1(js_ctx, 0x0a);
			DUK__EMIT_HSTR(js_ctx, h_stepback);
		}
	}
	DUK__EMIT_1(js_ctx, DUK_ASC_RBRACKET);

	duk__enc_objarr_exit(js_ctx, &h_stepback, &h_indent, &entry_top);

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
DUK_LOCAL duk_bool_t duk__enc_value1(duk_json_enc_ctx *js_ctx, duk_idx_t idx_holder) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_hobject *h;
	duk_tval *tv;
	duk_small_int_t c;

	DUK_DDD(DUK_DDDPRINT("duk__enc_value1: idx_holder=%ld, holder=%!T, key=%!T",
	                     (long) idx_holder, (duk_tval *) duk_get_tval(ctx, idx_holder),
	                     (duk_tval *) duk_get_tval(ctx, -1)));

	duk_dup_top(ctx);               /* -> [ ... key key ] */
	duk_get_prop(ctx, idx_holder);  /* -> [ ... key val ] */

	DUK_DDD(DUK_DDDPRINT("value=%!T", (duk_tval *) duk_get_tval(ctx, -1)));

	h = duk_get_hobject_or_lfunc_coerce(ctx, -1);
	if (h != NULL) {
		duk_get_prop_stridx(ctx, -1, DUK_STRIDX_TO_JSON);
		h = duk_get_hobject_or_lfunc_coerce(ctx, -1);  /* toJSON() can also be a lightfunc */

		if (h != NULL && DUK_HOBJECT_IS_CALLABLE(h)) {
			DUK_DDD(DUK_DDDPRINT("value is object, has callable toJSON() -> call it"));
			duk_dup(ctx, -2);         /* -> [ ... key val toJSON val ] */
			duk_dup(ctx, -4);         /* -> [ ... key val toJSON val key ] */
			duk_call_method(ctx, 1);  /* -> [ ... key val val' ] */
			duk_remove(ctx, -2);      /* -> [ ... key val' ] */
		} else {
			duk_pop(ctx);
		}
	}

	/* [ ... key val ] */

	DUK_DDD(DUK_DDDPRINT("value=%!T", (duk_tval *) duk_get_tval(ctx, -1)));

	if (js_ctx->h_replacer) {
		/* XXX: here a "slice copy" would be useful */
		DUK_DDD(DUK_DDDPRINT("replacer is set, call replacer"));
		duk_push_hobject(ctx, js_ctx->h_replacer);  /* -> [ ... key val replacer ] */
		duk_dup(ctx, idx_holder);                   /* -> [ ... key val replacer holder ] */
		duk_dup(ctx, -4);                           /* -> [ ... key val replacer holder key ] */
		duk_dup(ctx, -4);                           /* -> [ ... key val replacer holder key val ] */
		duk_call_method(ctx, 2);                    /* -> [ ... key val val' ] */
		duk_remove(ctx, -2);                        /* -> [ ... key val' ] */
	}

	/* [ ... key val ] */

	DUK_DDD(DUK_DDDPRINT("value=%!T", (duk_tval *) duk_get_tval(ctx, -1)));

	tv = duk_get_tval(ctx, -1);
	DUK_ASSERT(tv != NULL);
	if (DUK_TVAL_IS_OBJECT(tv)) {
		h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);

		c = (duk_small_int_t) DUK_HOBJECT_GET_CLASS_NUMBER(h);
		switch ((int) c) {
		case DUK_HOBJECT_CLASS_NUMBER:
			DUK_DDD(DUK_DDDPRINT("value is a Number object -> coerce with ToNumber()"));
			duk_to_number(ctx, -1);
			break;
		case DUK_HOBJECT_CLASS_STRING:
			DUK_DDD(DUK_DDDPRINT("value is a String object -> coerce with ToString()"));
			duk_to_string(ctx, -1);
			break;
#if defined(DUK_USE_JX) || defined(DUK_USE_JC)
		case DUK_HOBJECT_CLASS_BUFFER:
		case DUK_HOBJECT_CLASS_POINTER:
#endif
		case DUK_HOBJECT_CLASS_BOOLEAN:
			DUK_DDD(DUK_DDDPRINT("value is a Boolean/Buffer/Pointer object -> get internal value"));
			duk_get_prop_stridx(ctx, -1, DUK_STRIDX_INT_VALUE);
			duk_remove(ctx, -2);
			break;
		}
	}

	/* [ ... key val ] */

	DUK_DDD(DUK_DDDPRINT("value=%!T", (duk_tval *) duk_get_tval(ctx, -1)));

	if (duk_check_type_mask(ctx, -1, js_ctx->mask_for_undefined)) {
		/* will result in undefined */
		DUK_DDD(DUK_DDDPRINT("-> will result in undefined (type mask check)"));
		goto undef;
	}

	/* functions are detected specially */
	h = duk_get_hobject(ctx, -1);
	if (h != NULL && DUK_HOBJECT_IS_CALLABLE(h)) {
		if (js_ctx->flags & (DUK_JSON_FLAG_EXT_CUSTOM |
		                     DUK_JSON_FLAG_EXT_COMPATIBLE)) {
			/* function will be serialized to custom format */
		} else {
			/* functions are not serialized, results in undefined */
			DUK_DDD(DUK_DDDPRINT("-> will result in undefined (function)"));
			goto undef;
		}
	}

	DUK_DDD(DUK_DDDPRINT("-> will not result in undefined"));
	return 0;

 undef:
	duk_pop_2(ctx);
	return 1;
}

/* The Str(key, holder) operation: encode value, steps 5-10.
 *
 * This must not be called unless duk__enc_value1() returns non-zero.
 * If so, this is guaranteed to produce a non-undefined result.
 * Non-standard encodings (e.g. for undefined) are only used if
 * duk__enc_value1() indicates they are accepted; they're not
 * checked or asserted here again.
 *
 * Stack policy: [ ... key val ] -> [ ... ].
 */
DUK_LOCAL void duk__enc_value2(duk_json_enc_ctx *js_ctx) {
	duk_context *ctx = (duk_context *) js_ctx->thr;
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_tval *tv;

	DUK_UNREF(thr);

	DUK_DDD(DUK_DDDPRINT("duk__enc_value2: key=%!T, val=%!T",
	                     (duk_tval *) duk_get_tval(ctx, -2),
	                     (duk_tval *) duk_get_tval(ctx, -1)));

	/* [ ... key val ] */

	tv = duk_get_tval(ctx, -1);
	DUK_ASSERT(tv != NULL);

	switch (DUK_TVAL_GET_TAG(tv)) {
#if defined(DUK_USE_JX) || defined(DUK_USE_JC)
	/* When JX/JC not in use, duk__enc_value1 will block undefined values. */
	case DUK_TAG_UNDEFINED: {
		DUK__EMIT_STRIDX(js_ctx, js_ctx->stridx_custom_undefined);
		break;
	}
#endif
	case DUK_TAG_NULL: {
		DUK__EMIT_STRIDX(js_ctx, DUK_STRIDX_LC_NULL);
		break;
	}
	case DUK_TAG_BOOLEAN: {
		DUK__EMIT_STRIDX(js_ctx, DUK_TVAL_GET_BOOLEAN(tv) ?
		                 DUK_STRIDX_TRUE : DUK_STRIDX_FALSE);
		break;
	}
#if defined(DUK_USE_JX) || defined(DUK_USE_JC)
	/* When JX/JC not in use, duk__enc_value1 will block pointer values. */
	case DUK_TAG_POINTER: {
		char buf[64];  /* XXX: how to figure correct size? */
		const char *fmt;
		void *ptr = DUK_TVAL_GET_POINTER(tv);

		DUK_MEMZERO(buf, sizeof(buf));

		/* The #ifdef clutter here needs to handle the three cases:
		 * (1) JX+JC, (2) JX only, (3) JC only.
		 */
#if defined(DUK_USE_JX) && defined(DUK_USE_JC)
		if (js_ctx->flag_ext_custom)
#endif
#if defined(DUK_USE_JX)
		{
			fmt = ptr ? "(%p)" : "(null)";
		}
#endif
#if defined(DUK_USE_JX) && defined(DUK_USE_JC)
		else
#endif
#if defined(DUK_USE_JC)
		{
			fmt = ptr ? "{\"_ptr\":\"%p\"}" : "{\"_ptr\":\"null\"}";
		}
#endif

		/* When ptr == NULL, the format argument is unused. */
		DUK_SNPRINTF(buf, sizeof(buf) - 1, fmt, ptr);  /* must not truncate */
		DUK__EMIT_CSTR(js_ctx, buf);
		break;
	}
#endif  /* DUK_USE_JX || DUK_USE_JC */
	case DUK_TAG_STRING: {
		duk_hstring *h = DUK_TVAL_GET_STRING(tv);
		DUK_ASSERT(h != NULL);

		duk__enc_quote_string(js_ctx, h);
		break;
	}
	case DUK_TAG_OBJECT: {
		duk_hobject *h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);

#if defined(DUK_USE_JX) || defined(DUK_USE_JC)
		if (DUK_HOBJECT_IS_CALLABLE(h)) {
			/* We only get here when doing non-standard JSON encoding */
			DUK_ASSERT(js_ctx->flag_ext_custom || js_ctx->flag_ext_compatible);
			DUK__EMIT_STRIDX(js_ctx, js_ctx->stridx_custom_function);
		} else  /* continues below */
#endif
		if (DUK_HOBJECT_GET_CLASS_NUMBER(h) == DUK_HOBJECT_CLASS_ARRAY) {
			duk__enc_array(js_ctx);
		} else {
			duk__enc_object(js_ctx);
		}
		break;
	}
#if defined(DUK_USE_JX) || defined(DUK_USE_JC)
	/* When JX/JC not in use, duk__enc_value1 will block buffer values. */
	case DUK_TAG_BUFFER: {
		/* Buffer values are encoded in (lowercase) hex to make the
		 * binary data readable.  Base64 or similar would be more
		 * compact but less readable, and the point of JX/JC
		 * variants is to be as useful to a programmer as possible.
		 */

		/* The #ifdef clutter here needs to handle the three cases:
		 * (1) JX+JC, (2) JX only, (3) JC only.
		 */
#if defined(DUK_USE_JX) && defined(DUK_USE_JC)
		if (js_ctx->flag_ext_custom)
#endif
#if defined(DUK_USE_JX)
		{
			duk_uint8_t *p, *p_end;
			duk_small_uint_t x;
			duk_hbuffer *h;

			h = DUK_TVAL_GET_BUFFER(tv);
			DUK_ASSERT(h != NULL);
			p = (duk_uint8_t *) DUK_HBUFFER_GET_DATA_PTR(thr->heap, h);
			p_end = p + DUK_HBUFFER_GET_SIZE(h);
			DUK__EMIT_1(js_ctx, DUK_ASC_PIPE);
			while (p < p_end) {
				x = *p++;
				duk_hbuffer_append_byte(js_ctx->thr, js_ctx->h_buf, duk_lc_digits[(x >> 4) & 0x0f]);
				duk_hbuffer_append_byte(js_ctx->thr, js_ctx->h_buf, duk_lc_digits[x & 0x0f]);
			}
			DUK__EMIT_1(js_ctx, DUK_ASC_PIPE);
		}
#endif
#if defined(DUK_USE_JX) && defined(DUK_USE_JC)
		else
#endif
#if defined(DUK_USE_JC)
		{
			DUK_ASSERT(js_ctx->flag_ext_compatible);
			duk_hex_encode(ctx, -1);
			DUK__EMIT_CSTR(js_ctx, "{\"_buf\":");
			duk__enc_quote_string(js_ctx, duk_require_hstring(ctx, -1));
			DUK__EMIT_1(js_ctx, DUK_ASC_RCURLY);
		}
#endif
		break;
	}
#endif  /* DUK_USE_JX || DUK_USE_JC */
	case DUK_TAG_LIGHTFUNC: {
#if defined(DUK_USE_JX) || defined(DUK_USE_JC)
		/* We only get here when doing non-standard JSON encoding */
		DUK_ASSERT(js_ctx->flag_ext_custom || js_ctx->flag_ext_compatible);
		DUK__EMIT_STRIDX(js_ctx, js_ctx->stridx_custom_function);
#else
		/* Standard JSON omits functions */
		DUK_UNREACHABLE();
#endif
		break;
	}
#if defined(DUK_USE_FASTINT)
	case DUK_TAG_FASTINT:
#endif
	default: {
		/* number */
		duk_double_t d;
		duk_small_int_t c;
		duk_small_int_t s;
		duk_small_uint_t stridx;
		duk_small_uint_t n2s_flags;
		duk_hstring *h_str;

		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));
		d = DUK_TVAL_GET_NUMBER(tv);
		c = (duk_small_int_t) DUK_FPCLASSIFY(d);
		s = (duk_small_int_t) DUK_SIGNBIT(d);
		DUK_UNREF(s);

		if (DUK_LIKELY(!(c == DUK_FP_INFINITE || c == DUK_FP_NAN))) {
			DUK_ASSERT(DUK_ISFINITE(d));

#if defined(DUK_USE_JX) || defined(DUK_USE_JC)
			/* Negative zero needs special handling in JX/JC because
			 * it would otherwise serialize to '0', not '-0'.
			 */
			if (DUK_UNLIKELY(c == DUK_FP_ZERO && s != 0 &&
			                 (js_ctx->flag_ext_custom || js_ctx->flag_ext_compatible))) {
				duk_push_hstring_stridx(ctx, DUK_STRIDX_MINUS_ZERO);  /* '-0' */
			} else
#endif  /* DUK_USE_JX || DUK_USE_JC */
			{
				n2s_flags = 0;
				/* [ ... number ] -> [ ... string ] */
				duk_numconv_stringify(ctx, 10 /*radix*/, 0 /*digits*/, n2s_flags);
			}
			h_str = duk_to_hstring(ctx, -1);
			DUK_ASSERT(h_str != NULL);
			DUK__EMIT_HSTR(js_ctx, h_str);
			break;
		}

#if defined(DUK_USE_JX) || defined(DUK_USE_JC)
		if (!(js_ctx->flags & (DUK_JSON_FLAG_EXT_CUSTOM |
		                       DUK_JSON_FLAG_EXT_COMPATIBLE))) {
			stridx = DUK_STRIDX_LC_NULL;
		} else if (c == DUK_FP_NAN) {
			stridx = js_ctx->stridx_custom_nan;
		} else if (s == 0) {
			stridx = js_ctx->stridx_custom_posinf;
		} else {
			stridx = js_ctx->stridx_custom_neginf;
		}
#else
		stridx = DUK_STRIDX_LC_NULL;
#endif
		DUK__EMIT_STRIDX(js_ctx, stridx);
		break;
	}
	}

	/* [ ... key val ] -> [ ... ] */

	duk_pop_2(ctx);
}

/* E5 Section 15.12.3, main algorithm, step 4.b.ii steps 1-4. */
DUK_LOCAL duk_bool_t duk__enc_allow_into_proplist(duk_tval *tv) {
	duk_hobject *h;
	duk_small_int_t c;

	DUK_ASSERT(tv != NULL);
	if (DUK_TVAL_IS_STRING(tv) || DUK_TVAL_IS_NUMBER(tv)) {
		return 1;
	} else if (DUK_TVAL_IS_OBJECT(tv)) {
		h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);
		c = (duk_small_int_t) DUK_HOBJECT_GET_CLASS_NUMBER(h);
		if (c == DUK_HOBJECT_CLASS_STRING || c == DUK_HOBJECT_CLASS_NUMBER) {
			return 1;
		}
	}

	return 0;
}

/*
 *  Top level wrappers
 */

DUK_INTERNAL
void duk_bi_json_parse_helper(duk_context *ctx,
                              duk_idx_t idx_value,
                              duk_idx_t idx_reviver,
                              duk_small_uint_t flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_json_dec_ctx js_ctx_alloc;
	duk_json_dec_ctx *js_ctx = &js_ctx_alloc;
	duk_hstring *h_text;
#ifdef DUK_USE_ASSERTIONS
	duk_idx_t entry_top = duk_get_top(ctx);
#endif

	/* negative top-relative indices not allowed now */
	DUK_ASSERT(idx_value == DUK_INVALID_INDEX || idx_value >= 0);
	DUK_ASSERT(idx_reviver == DUK_INVALID_INDEX || idx_reviver >= 0);

	DUK_DDD(DUK_DDDPRINT("JSON parse start: text=%!T, reviver=%!T, flags=0x%08lx, stack_top=%ld",
	                     (duk_tval *) duk_get_tval(ctx, idx_value),
	                     (duk_tval *) duk_get_tval(ctx, idx_reviver),
	                     (unsigned long) flags,
	                     (long) duk_get_top(ctx)));

	DUK_MEMZERO(&js_ctx_alloc, sizeof(js_ctx_alloc));
	js_ctx->thr = thr;
#ifdef DUK_USE_EXPLICIT_NULL_INIT
	/* nothing now */
#endif
	js_ctx->recursion_limit = DUK_JSON_DEC_RECURSION_LIMIT;

	/* Flag handling currently assumes that flags are consistent.  This is OK
	 * because the call sites are now strictly controlled.
	 */

	js_ctx->flags = flags;
#ifdef DUK_USE_JX
	js_ctx->flag_ext_custom = flags & DUK_JSON_FLAG_EXT_CUSTOM;
#endif
#ifdef DUK_USE_JC
	js_ctx->flag_ext_compatible = flags & DUK_JSON_FLAG_EXT_COMPATIBLE;
#endif

	h_text = duk_to_hstring(ctx, idx_value);  /* coerce in-place */
	DUK_ASSERT(h_text != NULL);

	js_ctx->p_start = (duk_uint8_t *) DUK_HSTRING_GET_DATA(h_text);
	js_ctx->p = js_ctx->p_start;
	js_ctx->p_end = ((duk_uint8_t *) DUK_HSTRING_GET_DATA(h_text)) +
	                DUK_HSTRING_GET_BYTELEN(h_text);

	duk__dec_value(js_ctx);  /* -> [ ... value ] */

	/* Trailing whitespace has been eaten by duk__dec_value(), so if
	 * we're not at end of input here, it's a SyntaxError.
	 */

	if (js_ctx->p != js_ctx->p_end) {
		duk__dec_syntax_error(js_ctx);
	}

	if (duk_is_callable(ctx, idx_reviver)) {
		DUK_DDD(DUK_DDDPRINT("applying reviver: %!T",
		                     (duk_tval *) duk_get_tval(ctx, idx_reviver)));

		js_ctx->idx_reviver = idx_reviver;

		duk_push_object(ctx);
		duk_dup(ctx, -2);  /* -> [ ... val root val ] */
		duk_put_prop_stridx(ctx, -2, DUK_STRIDX_EMPTY_STRING);  /* default attrs ok */
		duk_push_hstring_stridx(ctx, DUK_STRIDX_EMPTY_STRING);  /* -> [ ... val root "" ] */

		DUK_DDD(DUK_DDDPRINT("start reviver walk, root=%!T, name=%!T",
		                     (duk_tval *) duk_get_tval(ctx, -2),
		                     (duk_tval *) duk_get_tval(ctx, -1)));

		duk__dec_reviver_walk(js_ctx);  /* [ ... val root "" ] -> [ ... val val' ] */
		duk_remove(ctx, -2);            /* -> [ ... val' ] */
	} else {
		DUK_DDD(DUK_DDDPRINT("reviver does not exist or is not callable: %!T",
		                     (duk_tval *) duk_get_tval(ctx, idx_reviver)));
	}

	/* Final result is at stack top. */

	DUK_DDD(DUK_DDDPRINT("JSON parse end: text=%!T, reviver=%!T, flags=0x%08lx, result=%!T, stack_top=%ld",
	                     (duk_tval *) duk_get_tval(ctx, idx_value),
	                     (duk_tval *) duk_get_tval(ctx, idx_reviver),
	                     (unsigned long) flags,
	                     (duk_tval *) duk_get_tval(ctx, -1),
	                     (long) duk_get_top(ctx)));

	DUK_ASSERT(duk_get_top(ctx) == entry_top + 1);
}

DUK_INTERNAL
void duk_bi_json_stringify_helper(duk_context *ctx,
                                  duk_idx_t idx_value,
                                  duk_idx_t idx_replacer,
                                  duk_idx_t idx_space,
                                  duk_small_uint_t flags) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_json_enc_ctx js_ctx_alloc;
	duk_json_enc_ctx *js_ctx = &js_ctx_alloc;
	duk_hobject *h;
	duk_bool_t undef;
	duk_idx_t idx_holder;
	duk_idx_t entry_top;

	/* negative top-relative indices not allowed now */
	DUK_ASSERT(idx_value == DUK_INVALID_INDEX || idx_value >= 0);
	DUK_ASSERT(idx_replacer == DUK_INVALID_INDEX || idx_replacer >= 0);
	DUK_ASSERT(idx_space == DUK_INVALID_INDEX || idx_space >= 0);

	DUK_DDD(DUK_DDDPRINT("JSON stringify start: value=%!T, replacer=%!T, space=%!T, flags=0x%08lx, stack_top=%ld",
	                     (duk_tval *) duk_get_tval(ctx, idx_value),
	                     (duk_tval *) duk_get_tval(ctx, idx_replacer),
	                     (duk_tval *) duk_get_tval(ctx, idx_space),
	                     (unsigned long) flags,
	                     (long) duk_get_top(ctx)));

	entry_top = duk_get_top(ctx);

	/*
	 *  Context init
	 */

	DUK_MEMZERO(&js_ctx_alloc, sizeof(js_ctx_alloc));
	js_ctx->thr = thr;
#ifdef DUK_USE_EXPLICIT_NULL_INIT
	js_ctx->h_replacer = NULL;
	js_ctx->h_gap = NULL;
	js_ctx->h_indent = NULL;
#endif
	js_ctx->idx_proplist = -1;
	js_ctx->recursion_limit = DUK_JSON_ENC_RECURSION_LIMIT;

	/* Flag handling currently assumes that flags are consistent.  This is OK
	 * because the call sites are now strictly controlled.
	 */

	js_ctx->flags = flags;
	js_ctx->flag_ascii_only = flags & DUK_JSON_FLAG_ASCII_ONLY;
	js_ctx->flag_avoid_key_quotes = flags & DUK_JSON_FLAG_AVOID_KEY_QUOTES;
#ifdef DUK_USE_JX
	js_ctx->flag_ext_custom = flags & DUK_JSON_FLAG_EXT_CUSTOM;
#endif
#ifdef DUK_USE_JC
	js_ctx->flag_ext_compatible = flags & DUK_JSON_FLAG_EXT_COMPATIBLE;
#endif

	/* The #ifdef clutter here handles the JX/JC enable/disable
	 * combinations properly.
	 */
#if defined(DUK_USE_JX) || defined(DUK_USE_JC)
#if defined(DUK_USE_JX)
	if (flags & DUK_JSON_FLAG_EXT_CUSTOM) {
		js_ctx->stridx_custom_undefined = DUK_STRIDX_LC_UNDEFINED;
		js_ctx->stridx_custom_nan = DUK_STRIDX_NAN;
		js_ctx->stridx_custom_neginf = DUK_STRIDX_MINUS_INFINITY;
		js_ctx->stridx_custom_posinf = DUK_STRIDX_INFINITY;
		js_ctx->stridx_custom_function =
		        (flags & DUK_JSON_FLAG_AVOID_KEY_QUOTES) ?
		                DUK_STRIDX_JSON_EXT_FUNCTION2 :
		                DUK_STRIDX_JSON_EXT_FUNCTION1;
	}
#endif  /* DUK_USE_JX */
#if defined(DUK_USE_JX) && defined(DUK_USE_JC)
	else
#endif  /* DUK_USE_JX && DUK_USE_JC */
#if defined(DUK_USE_JC)
	if (js_ctx->flags & DUK_JSON_FLAG_EXT_COMPATIBLE) {
		js_ctx->stridx_custom_undefined = DUK_STRIDX_JSON_EXT_UNDEFINED;
		js_ctx->stridx_custom_nan = DUK_STRIDX_JSON_EXT_NAN;
		js_ctx->stridx_custom_neginf = DUK_STRIDX_JSON_EXT_NEGINF;
		js_ctx->stridx_custom_posinf = DUK_STRIDX_JSON_EXT_POSINF;
		js_ctx->stridx_custom_function = DUK_STRIDX_JSON_EXT_FUNCTION1;
	}
#endif  /* DUK_USE_JC */
#endif  /* DUK_USE_JX || DUK_USE_JC */

#if defined(DUK_USE_JX) || defined(DUK_USE_JC)
	if (js_ctx->flags & (DUK_JSON_FLAG_EXT_CUSTOM |
	                     DUK_JSON_FLAG_EXT_COMPATIBLE)) {
		DUK_ASSERT(js_ctx->mask_for_undefined == 0);  /* already zero */
	}
	else
#endif  /* DUK_USE_JX || DUK_USE_JC */
	{
		js_ctx->mask_for_undefined = DUK_TYPE_MASK_UNDEFINED |
		                             DUK_TYPE_MASK_POINTER |
		                             DUK_TYPE_MASK_BUFFER |
		                             DUK_TYPE_MASK_LIGHTFUNC;
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

			duk_uarridx_t plist_idx = 0;
			duk_small_uint_t enum_flags;

			js_ctx->idx_proplist = duk_push_array(ctx);  /* XXX: array internal? */

			enum_flags = DUK_ENUM_ARRAY_INDICES_ONLY |
			             DUK_ENUM_SORT_ARRAY_INDICES;  /* expensive flag */
			duk_enum(ctx, idx_replacer, enum_flags);
			while (duk_next(ctx, -1 /*enum_index*/, 1 /*get_value*/)) {
				/* [ ... proplist enum_obj key val ] */
				if (duk__enc_allow_into_proplist(duk_get_tval(ctx, -1))) {
					/* XXX: duplicates should be eliminated here */
					DUK_DDD(DUK_DDDPRINT("proplist enum: key=%!T, val=%!T --> accept",
					                     (duk_tval *) duk_get_tval(ctx, -2),
					                     (duk_tval *) duk_get_tval(ctx, -1)));
					duk_to_string(ctx, -1);  /* extra coercion of strings is OK */
					duk_put_prop_index(ctx, -4, plist_idx);  /* -> [ ... proplist enum_obj key ] */
					plist_idx++;
					duk_pop(ctx);
				} else {
					DUK_DDD(DUK_DDDPRINT("proplist enum: key=%!T, val=%!T --> reject",
					                     (duk_tval *) duk_get_tval(ctx, -2),
					                     (duk_tval *) duk_get_tval(ctx, -1)));
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
		duk_small_int_t nspace;
		/* spaces[] must be static to allow initializer with old compilers like BCC */
		static const char spaces[10] = {
			DUK_ASC_SPACE, DUK_ASC_SPACE, DUK_ASC_SPACE, DUK_ASC_SPACE,
			DUK_ASC_SPACE, DUK_ASC_SPACE, DUK_ASC_SPACE, DUK_ASC_SPACE,
			DUK_ASC_SPACE, DUK_ASC_SPACE
		};  /* XXX: helper */

		/* ToInteger() coercion; NaN -> 0, infinities are clamped to 0 and 10 */
		nspace = (duk_small_int_t) duk_to_int_clamped(ctx, idx_space, 0 /*minval*/, 10 /*maxval*/);
		DUK_ASSERT(nspace >= 0 && nspace <= 10);

		duk_push_lstring(ctx, spaces, (duk_size_t) nspace);
		js_ctx->h_gap = duk_get_hstring(ctx, -1);
		DUK_ASSERT(js_ctx->h_gap != NULL);
	} else if (duk_is_string(ctx, idx_space)) {
		/* XXX: substring in-place at idx_place? */
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

	DUK_DDD(DUK_DDDPRINT("before: flags=0x%08lx, buf=%!O, loop=%!T, replacer=%!O, "
	                     "proplist=%!T, gap=%!O, indent=%!O, holder=%!T",
	                     (unsigned long) js_ctx->flags,
	                     (duk_heaphdr *) js_ctx->h_buf,
	                     (duk_tval *) duk_get_tval(ctx, js_ctx->idx_loop),
	                     (duk_heaphdr *) js_ctx->h_replacer,
	                     (duk_tval *) (js_ctx->idx_proplist >= 0 ? duk_get_tval(ctx, js_ctx->idx_proplist) : NULL),
	                     (duk_heaphdr *) js_ctx->h_gap,
	                     (duk_heaphdr *) js_ctx->h_indent,
	                     (duk_tval *) duk_get_tval(ctx, -1)));

	/* serialize the wrapper with empty string key */

	duk_push_hstring_stridx(ctx, DUK_STRIDX_EMPTY_STRING);

	/* [ ... buf loop (proplist) (gap) holder "" ] */

	undef = duk__enc_value1(js_ctx, idx_holder);  /* [ ... holder key ] -> [ ... holder key val ] */

	DUK_DDD(DUK_DDDPRINT("after: flags=0x%08lx, buf=%!O, loop=%!T, replacer=%!O, "
	                     "proplist=%!T, gap=%!O, indent=%!O, holder=%!T",
	                     (unsigned long) js_ctx->flags,
	                     (duk_heaphdr *) js_ctx->h_buf,
	                     (duk_tval *) duk_get_tval(ctx, js_ctx->idx_loop),
	                     (duk_heaphdr *) js_ctx->h_replacer,
	                     (duk_tval *) (js_ctx->idx_proplist >= 0 ? duk_get_tval(ctx, js_ctx->idx_proplist) : NULL),
	                     (duk_heaphdr *) js_ctx->h_gap,
	                     (duk_heaphdr *) js_ctx->h_indent,
	                     (duk_tval *) duk_get_tval(ctx, -3)));

	if (undef) {
		/*
		 *  Result is undefined
		 */

		duk_push_undefined(ctx);
	} else {
		/*
		 *  Finish and convert buffer to result string
		 */

		duk__enc_value2(js_ctx);  /* [ ... key val ] -> [ ... ] */
		DUK_ASSERT(js_ctx->h_buf != NULL);
		duk_push_hbuffer(ctx, (duk_hbuffer *) js_ctx->h_buf);
		duk_to_string(ctx, -1);
	}

	/* The stack has a variable shape here, so force it to the
	 * desired one explicitly.
	 */

	duk_replace(ctx, entry_top);
	duk_set_top(ctx, entry_top + 1);

	DUK_DDD(DUK_DDDPRINT("JSON stringify end: value=%!T, replacer=%!T, space=%!T, "
	                     "flags=0x%08lx, result=%!T, stack_top=%ld",
	                     (duk_tval *) duk_get_tval(ctx, idx_value),
	                     (duk_tval *) duk_get_tval(ctx, idx_replacer),
	                     (duk_tval *) duk_get_tval(ctx, idx_space),
	                     (unsigned long) flags,
	                     (duk_tval *) duk_get_tval(ctx, -1),
	                     (long) duk_get_top(ctx)));

	DUK_ASSERT(duk_get_top(ctx) == entry_top + 1);
}

/*
 *  Entry points
 */

DUK_INTERNAL duk_ret_t duk_bi_json_object_parse(duk_context *ctx) {
	duk_bi_json_parse_helper(ctx,
	                         0 /*idx_value*/,
	                         1 /*idx_replacer*/,
	                         0 /*flags*/);
	return 1;
}

DUK_INTERNAL duk_ret_t duk_bi_json_object_stringify(duk_context *ctx) {
	duk_bi_json_stringify_helper(ctx,
	                             0 /*idx_value*/,
	                             1 /*idx_replacer*/,
	                             2 /*idx_space*/,
	                             0 /*flags*/);
	return 1;
}
