/*
 *  JSON built-ins.
 *
 *  See doc/json.txt.
 */

#include "duk_internal.h"

/*
 *  Local defines and forward declarations
 */

static void json_emit_1(duk_json_enc_ctx *js_ctx, char ch);
static void json_emit_2(duk_json_enc_ctx *js_ctx, int chars);
static void json_emit_esc(duk_json_enc_ctx *js_ctx, duk_u32 cp, char *esc_str, int digits);
static void json_emit_esc16(duk_json_enc_ctx *js_ctx, duk_u32 cp);
static void json_emit_esc32(duk_json_enc_ctx *js_ctx, duk_u32 cp);
static void json_emit_xutf8(duk_json_enc_ctx *js_ctx, duk_u32 cp);
static void json_emit_hstring(duk_json_enc_ctx *js_ctx, duk_hstring *h);
static void json_emit_cstring(duk_json_enc_ctx *js_ctx, const char *p);
static int json_key_quotes_needed(duk_hstring *h_key);
static void json_quote_string(duk_json_enc_ctx *js_ctx, duk_hstring *h_str);
static void json_enc_objarr_shared_entry(duk_json_enc_ctx *js_ctx, duk_hstring **h_stepback, duk_hstring **h_indent, int *entry_top);
static void json_enc_objarr_shared_exit(duk_json_enc_ctx *js_ctx, duk_hstring **h_stepback, duk_hstring **h_indent, int *entry_top);
static void json_enc_object(duk_json_enc_ctx *js_ctx);
static void json_enc_array(duk_json_enc_ctx *js_ctx);
static int json_enc_value1(duk_json_enc_ctx *js_ctx, int idx_holder);
static void json_enc_value2(duk_json_enc_ctx *js_ctx);
static int json_enc_allow_into_proplist(duk_tval *tv);

/*
 *  Parsing implementation.
 */

/*FIXME*/

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
		duk_hbuffer_append_byte(js_ctx->thr, js_ctx->h_buf, duk_nybbles[dig]);
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
static int json_key_quotes_needed(duk_hstring *h_key) {
	duk_u8 *p, *p_start, *p_end;
	int ch;

	DUK_ASSERT(h_key != NULL);
	p_start = DUK_HSTRING_GET_DATA(h_key);
	p_end = p_start + DUK_HSTRING_GET_BYTELEN(h_key);
	p = p_start;

	DUK_DPRINT("json_key_quotes_needed: h_key=%!O, p_start=%p, p_end=%p, p=%p",
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

static void json_quote_string(duk_json_enc_ctx *js_ctx, duk_hstring *h_str) {
	duk_hthread *thr = js_ctx->thr;
	duk_u8 *p, *p_start, *p_end;
	duk_u32 cp;

	DUK_DPRINT("json_quote_string: h_str=%!O", h_str);

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
			cp = duk_unicode_xutf8_get_u32(thr, &p, p_start, p_end);
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
	duk_push_true(ctx);  /* -> [ ... voidp true ] */
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

	DUK_DPRINT("shared entry finished: top=%d, loop=%!T",
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

	DUK_DPRINT("shared entry finished: top=%d, loop=%!T",
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

	DUK_DPRINT("json_enc_object: obj=%!T", duk_get_tval(ctx, -1));

	json_enc_objarr_shared_entry(js_ctx, &h_stepback, &h_indent, &entry_top);

	idx_obj = entry_top - 1;

	if (js_ctx->idx_proplist >= 0) {
		idx_keys = js_ctx->idx_proplist;
	} else {
		/* FIXME: would be nice to enumerate an object at specified index */
		duk_dup(ctx, idx_obj);
		duk_hobject_get_enumerated_keys(ctx, DUK_ENUM_OWN_PROPERTIES_ONLY /*flags*/);  /* [ ... target ] -> [ ... target keys ] */
		idx_keys = duk_require_normalize_index(ctx, -1);
		/* leave stack unbalanced on purpose */
	}

	DUK_DPRINT("idx_keys=%d, h_keys=%!T", idx_keys, duk_get_tval(ctx, idx_keys));

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

		DUK_DPRINT("object property loop: holder=%!T, key=%!T",
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
		if (js_ctx->flag_avoid_key_quotes && !json_key_quotes_needed(h_key)) {
			/* emit key as is */
			EMIT_HSTR(js_ctx, h_key);
		} else {
			json_quote_string(js_ctx, h_key);
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

	DUK_ASSERT(duk_get_top(ctx) == entry_top);
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
	int i;
	int arr_len;  /* FIXME: type */

	DUK_DPRINT("json_enc_array: array=%!T", duk_get_tval(ctx, -1));

	json_enc_objarr_shared_entry(js_ctx, &h_stepback, &h_indent, &entry_top);

	idx_arr = entry_top - 1;

	/* Steps 8-10 have been merged to avoid a "partial" variable. */

	EMIT_1(js_ctx, '[');

	arr_len = duk_get_length(ctx, idx_arr);
	for (i = 0; i < arr_len; i++) {
		DUK_DPRINT("array entry loop: array=%!T, h_indent=%!O, h_stepback=%!O, index=%d, arr_len=%d",
		           duk_get_tval(ctx, idx_arr), h_indent, h_stepback, i, arr_len);

		if (i > 0) {
			EMIT_1(js_ctx, ',');
		}
		if (h_indent != NULL) {
			EMIT_1(js_ctx, (char) 0x0a);
			EMIT_HSTR(js_ctx, h_indent);
		}

		/* FIXME: duk_push_int_string() */
		duk_push_int(ctx, i);
		duk_to_string(ctx, -1);  /* -> [ ... key ] */
		undef = json_enc_value1(js_ctx, idx_arr);

		if (undef) {
			EMIT_STRIDX(js_ctx, DUK_HEAP_STRIDX_NULL);
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

	DUK_ASSERT(duk_get_top(ctx) == entry_top);
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

	DUK_DPRINT("json_enc_value1: idx_holder=%d, holder=%!T, key=%!T",
	           idx_holder, duk_get_tval(ctx, idx_holder), duk_get_tval(ctx, -1));

	duk_dup_top(ctx);               /* -> [ ... key key ] */
	duk_get_prop(ctx, idx_holder);  /* -> [ ... key val ] */

	DUK_DPRINT("value=%!T", duk_get_tval(ctx, -1));

	h = duk_get_hobject(ctx, -1);
	if (h != NULL) {
		duk_get_prop_stridx(ctx, -1, DUK_HEAP_STRIDX_TO_JSON);
		h = duk_get_hobject(ctx, -1);  /* FIXME: duk_get_hobject_callable */
		if (h != NULL && DUK_HOBJECT_IS_CALLABLE(h)) {
			DUK_DPRINT("value is object, has callable toJSON() -> call it");
			duk_dup(ctx, -2);         /* -> [ ... key val toJSON val ] */
			duk_dup(ctx, -4);         /* -> [ ... key val toJSON val key ] */
			duk_call_method(ctx, 1);  /* -> [ ... key val val' ] */
			duk_remove(ctx, -2);      /* -> [ ... key val' ] */
		} else {
			duk_pop(ctx);
		}
	}

	/* [ ... key val ] */

	DUK_DPRINT("value=%!T", duk_get_tval(ctx, -1));

	if (js_ctx->h_replacer) {
		/* FIXME: here a "slice copy" would be useful */
		DUK_DPRINT("replacer is set, call replacer");
		duk_push_hobject(ctx, js_ctx->h_replacer);  /* -> [ ... key val replacer ] */
		duk_dup(ctx, idx_holder);                   /* -> [ ... key val replacer holder ] */
		duk_dup(ctx, -4);                           /* -> [ ... key val replacer holder key ] */
		duk_dup(ctx, -4);                           /* -> [ ... key val replacer holder key val ] */
		duk_call_method(ctx, 2);                    /* -> [ ... key val val' ] */
		duk_remove(ctx, -2);                        /* -> [ ... key val' ] */
	}

	/* [ ... key val ] */

	DUK_DPRINT("value=%!T", duk_get_tval(ctx, -1));

	tv = duk_get_tval(ctx, -1);
	DUK_ASSERT(tv != NULL);
	if (DUK_TVAL_IS_OBJECT(tv)) {
		h = DUK_TVAL_GET_OBJECT(tv);
		DUK_ASSERT(h != NULL);

		c = DUK_HOBJECT_GET_CLASS_NUMBER(h);
		if (c == DUK_HOBJECT_CLASS_NUMBER) {
			DUK_DPRINT("value is a Number object -> coerce with ToNumber()");
			duk_to_number(ctx, -1);
		} else if (c == DUK_HOBJECT_CLASS_STRING) {
			DUK_DPRINT("value is a String object -> coerce with ToString()");
			duk_to_string(ctx, -1);
		} else if (c == DUK_HOBJECT_CLASS_BOOLEAN) {
			DUK_DPRINT("value is a Boolean object -> get internal value");
			duk_get_prop_stridx(ctx, -1, DUK_HEAP_STRIDX_INT_VALUE);
			DUK_ASSERT(DUK_TVAL_IS_BOOLEAN(duk_get_tval(ctx, -1)));
			duk_remove(ctx, -2);
		}
	}

	/* [ ... key val ] */

	DUK_DPRINT("value=%!T", duk_get_tval(ctx, -1));

	tv = duk_get_tval(ctx, -1);
	DUK_ASSERT(tv != NULL);

	if (duk_get_type_mask(ctx, -1) & js_ctx->mask_for_undefined) {
		/* will result in undefined */
		DUK_DPRINT("-> will result in undefined (type mask check)");
		goto undef;
	}

	h = duk_get_hobject(ctx, -1);
	if (h != NULL && DUK_HOBJECT_IS_CALLABLE(h)) {
		if (js_ctx->flags & (DUK_JSON_ENC_FLAG_EXT_CUSTOM |
		                     DUK_JSON_ENC_FLAG_EXT_COMPATIBLE)) {
			/* function will be serialized to custom format */
		} else {
			/* functions are not serialized, results in undefined */
			DUK_DPRINT("-> will result in undefined (function)");
			goto undef;
		}
	}

	DUK_DPRINT("-> will not result in undefined");
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

	DUK_DPRINT("json_enc_value2: key=%!T, val=%!T",
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
		EMIT_STRIDX(js_ctx, DUK_HEAP_STRIDX_NULL);
		break;
	}
	case DUK_TAG_BOOLEAN: {
		EMIT_STRIDX(js_ctx, DUK_TVAL_GET_BOOLEAN(tv) ?
		            DUK_HEAP_STRIDX_TRUE : DUK_HEAP_STRIDX_FALSE);
		break;
	}
	case DUK_TAG_POINTER: {
		char buf[40];  /* FIXME: how to figure correct size? */
		const char *fmt;

		/* FIXME: NULL results in '((nil))' now */
		memset(buf, 0, sizeof(buf));
		if (js_ctx->flag_ext_custom) {
			fmt = "(%p)";
		} else {
			DUK_ASSERT(js_ctx->flag_ext_compatible);
			fmt = "{\"_ptr\":\"(%p)\"}";
		}
		snprintf(buf, sizeof(buf) - 1, fmt, (void *) DUK_TVAL_GET_POINTER(tv));
		EMIT_CSTR(js_ctx, buf);
		break;
	}
	case DUK_TAG_STRING: {
		duk_hstring *h = DUK_TVAL_GET_STRING(tv);
		DUK_ASSERT(h != NULL);

		json_quote_string(js_ctx, h);
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
			json_quote_string(js_ctx, duk_require_hstring(ctx, -1));
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
				duk_hbuffer_append_byte(js_ctx->thr, js_ctx->h_buf, duk_nybbles[(x >> 4) & 0x0f]);
				duk_hbuffer_append_byte(js_ctx->thr, js_ctx->h_buf, duk_nybbles[x & 0x0f]);
			}
			EMIT_1(js_ctx, '|');
		} else {
			DUK_ASSERT(js_ctx->flag_ext_compatible);
			duk_base64_encode(ctx, -1);
			EMIT_CSTR(js_ctx, "{\"_base64\":");  /* FIXME: stridx */
			json_quote_string(js_ctx, duk_require_hstring(ctx, -1));
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
		duk_hstring *h_str;
		DUK_ASSERT(DUK_TVAL_IS_NUMBER(tv));

		d = DUK_TVAL_GET_NUMBER(tv);
		c = fpclassify(d);
		s = signbit(d);

		if (!(c == FP_INFINITE || c == FP_NAN)) {
			DUK_ASSERT(isfinite(d));
			h_str = duk_to_hstring(ctx, -1);
			DUK_ASSERT(h_str != NULL);
			EMIT_HSTR(js_ctx, h_str);
			break;
		}

		/*FIXME:awkward check*/
		if (!(js_ctx->flags & (DUK_JSON_ENC_FLAG_EXT_CUSTOM |
		                       DUK_JSON_ENC_FLAG_EXT_COMPATIBLE))) {
			stridx = DUK_HEAP_STRIDX_NULL;
		} else if (c == FP_NAN) {
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
 *  Entry points
 */

int duk_builtin_json_object_parse(duk_context *ctx) {
	return DUK_RET_UNIMPLEMENTED_ERROR;	/*FIXME*/
}

int duk_builtin_json_object_stringify(duk_context *ctx) {
	duk_hthread *thr = (duk_hthread *) ctx;
	duk_json_enc_ctx js_ctx_alloc;
	duk_json_enc_ctx *js_ctx = &js_ctx_alloc;
	duk_hobject *h;
	int undef;
	int flags;
	int idx_holder;

	/* FIXME: extract a generic helper */

	DUK_DPRINT("JSON.stringify() start: value=%!T, replacer=%!T, space=%!T, stack_top=%d",
	           duk_get_tval(ctx, 0), duk_get_tval(ctx, 1),
	           duk_get_tval(ctx, 2), duk_get_top(ctx));

	/*
	 *  Context init
	 */

	memset(&js_ctx_alloc, 0, sizeof(js_ctx_alloc));
	js_ctx->thr = thr;
#ifdef DUK_USE_EXPLICIT_NULL_INIT
	js_ctx->h_replacer = NULL;
	js_ctx->h_gap = NULL;
	js_ctx->h_indent = NULL;
#endif
	js_ctx->idx_proplist = -1;
	js_ctx->recursion_limit = DUK_JSON_ENC_RECURSION_LIMIT;

	flags = 0;
#if 0
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
		js_ctx->stridx_custom_undefined = DUK_HEAP_STRIDX_UNDEFINED;
		js_ctx->stridx_custom_nan = DUK_HEAP_STRIDX_NAN;
		js_ctx->stridx_custom_neginf = DUK_HEAP_STRIDX_MINUS_INFINITY;
		js_ctx->stridx_custom_posinf = DUK_HEAP_STRIDX_INFINITY;
	} else if (js_ctx->flags & DUK_JSON_ENC_FLAG_EXT_COMPATIBLE) {
		js_ctx->stridx_custom_undefined = DUK_HEAP_STRIDX_JSON_EXT_UNDEFINED;
		js_ctx->stridx_custom_nan = DUK_HEAP_STRIDX_JSON_EXT_NAN;
		js_ctx->stridx_custom_neginf = DUK_HEAP_STRIDX_JSON_EXT_NEGINF;
		js_ctx->stridx_custom_posinf = DUK_HEAP_STRIDX_JSON_EXT_POSINF;
	}

	if (js_ctx->flags & (DUK_JSON_ENC_FLAG_EXT_CUSTOM |
	                     DUK_JSON_ENC_FLAG_EXT_COMPATIBLE)) {
		DUK_ASSERT(js_ctx->mask_for_undefined == 0);  /* already zero */
	} else {
		js_ctx->mask_for_undefined = DUK_TYPE_MASK_UNDEFINED |
		                             DUK_TYPE_MASK_POINTER |
		                             DUK_TYPE_MASK_BUFFER;
	}

	(void) duk_push_new_growable_buffer(ctx, 0);
	js_ctx->h_buf = (duk_hbuffer_growable *) duk_get_hbuffer(ctx, -1);
	DUK_ASSERT(js_ctx->h_buf != NULL);
	DUK_ASSERT(DUK_HBUFFER_HAS_GROWABLE(js_ctx->h_buf));

	js_ctx->idx_loop = duk_push_new_object_internal(ctx);
	DUK_ASSERT(js_ctx->idx_loop >= 0);

	/*
	 *  Process replacer/proplist (2nd argument)
	 */

	h = duk_get_hobject(ctx, 1);
	if (h != NULL) {
		if (DUK_HOBJECT_IS_CALLABLE(h)) {
			js_ctx->h_replacer = h;
		} else if (DUK_HOBJECT_GET_CLASS_NUMBER(h) == DUK_HOBJECT_CLASS_ARRAY) {
			/* Here the specification requires correct array index enumeration.
			 * We don't currently fulfill that requirement for sparse arrays,
			 * as that would be quite complicated and/or slow to do.  We now
			 * enumerate ancestors too, although the specification is not very
			 * clear on whether that is required.
			 */

			int plist_idx = 0;

			js_ctx->idx_proplist = duk_push_new_array(ctx);  /* FIXME: array internal? */

			/*FIXME:enum API*/
			duk_enum(ctx, 1, DUK_ENUM_ARRAY_INDICES_ONLY /*flags*/);
			while (duk_next(ctx, -1 /*enum_index*/, 1 /*get_value*/)) {
				/* [ ... proplist enum_obj key val ] */
				if (json_enc_allow_into_proplist(duk_get_tval(ctx, -1))) {
					/* FIXME: duplicates should be eliminated here */
					DUK_DPRINT("proplist enum: key=%!T, val=%!T --> accept", duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));
					duk_to_string(ctx, -1);  /* extra coercion of strings is OK */
					duk_put_prop_index(ctx, -4, plist_idx);  /* -> [ ... proplist enum_obj key ] */
					plist_idx++;
					duk_pop(ctx);
				} else {
					DUK_DPRINT("proplist enum: key=%!T, val=%!T --> reject", duk_get_tval(ctx, -2), duk_get_tval(ctx, -1));
					duk_pop_2(ctx);
				}
                        }
			/* FIXME: enum API should leave this empty */
                        duk_pop_3(ctx);  /* pop enum, key, value */

			/* [ ... proplist ] */
		}
	}

	/*
	 *  Process space (3rd argument)
	 */

	h = duk_get_hobject(ctx, 2);
	if (h != NULL) {
		int c = DUK_HOBJECT_GET_CLASS_NUMBER(h);
		if (c == DUK_HOBJECT_CLASS_NUMBER) {
			duk_to_number(ctx, 2);
		} else if (c == DUK_HOBJECT_CLASS_STRING) {
			duk_to_string(ctx, 2);
		}
	}

	if (duk_is_number(ctx, 2)) {
		double d;
		int nspace;
		char spaces[10] = { ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ' };  /*FIXME:helper*/

		/* ToInteger() coercion; NaN -> 0, infinities are clamped to 0 and 10 */
		/* FIXME: get_clamped_int */
		(void) duk_to_int(ctx, 2);
		d = duk_get_number(ctx, 2);
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
	} else if (duk_is_string(ctx, 2)) {
		/* FIXME: substring API requires a dup */
		duk_dup(ctx, 2);
		duk_substring(ctx, 0, 10);  /* clamp to 10 chars */
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

	/*
	 *  Create wrapper object and serialize
	 */

	idx_holder = duk_push_new_object(ctx);
	duk_dup(ctx, 0);
	duk_put_prop_stridx(ctx, -2, DUK_HEAP_STRIDX_EMPTY_STRING);

	DUK_DPRINT("before: flags=0x%08x, buf=%!O, loop=%!T, replacer=%!O, proplist=%!T, gap=%!O, indent=%!O, holder=%!T",
	           js_ctx->flags,
	           js_ctx->h_buf,
	           duk_get_tval(ctx, js_ctx->idx_loop),
	           js_ctx->h_replacer,
	           js_ctx->idx_proplist >= 0 ? duk_get_tval(ctx, js_ctx->idx_proplist) : NULL,
	           js_ctx->h_gap,
	           js_ctx->h_indent,
	           duk_get_tval(ctx, -1));
	
	/* serialize the wrapper with empty string key */

	duk_push_hstring_stridx(ctx, DUK_HEAP_STRIDX_EMPTY_STRING);
	undef = json_enc_value1(js_ctx, idx_holder);  /* [ ... key ] -> [ ... key val ] */
	if (undef) {
		return 0;
	} else {
		json_enc_value2(js_ctx);  /* [ ... key val ] -> [ ... ] */
	}

	DUK_DPRINT("after: flags=0x%08x, buf=%!O, loop=%!T, replacer=%!O, proplist=%!T, gap=%!O, indent=%!O, holder=%!T",
	           js_ctx->flags,
	           js_ctx->h_buf,
	           duk_get_tval(ctx, js_ctx->idx_loop),
	           js_ctx->h_replacer,
	           js_ctx->idx_proplist >= 0 ? duk_get_tval(ctx, js_ctx->idx_proplist) : NULL,
	           js_ctx->h_gap,
	           js_ctx->h_indent,
	           duk_get_tval(ctx, -1));

	/*
	 *  Convert buffer to result string
	 */

	DUK_ASSERT(js_ctx->h_buf != NULL);
	duk_push_hbuffer(ctx, (duk_hbuffer *) js_ctx->h_buf);
	duk_to_string(ctx, -1);

	DUK_DPRINT("JSON.stringify() end: value=%!T, replacer=%!T, space=%!T, stack_top=%d",
	           duk_get_tval(ctx, 0), duk_get_tval(ctx, 1),
	           duk_get_tval(ctx, 2), duk_get_top(ctx));

	return 1;
}

