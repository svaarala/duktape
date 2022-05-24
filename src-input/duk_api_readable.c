#include "duk_internal.h"

DUK_LOCAL const char * const duk__symbol_type_strings[4] = { "hidden", "global", "local", "wellknown" };

DUK_LOCAL duk_small_uint_t duk__get_symbol_type(duk_hstring *h) {
	const duk_uint8_t *data;
	duk_size_t blen;

	DUK_ASSERT(h != NULL);
	DUK_ASSERT(DUK_HSTRING_HAS_SYMBOL(h));
	DUK_ASSERT(duk_hstring_get_bytelen(h) >= 1); /* always true, symbol prefix */

	data = (const duk_uint8_t *) duk_hstring_get_data_and_bytelen(h, &blen);
	DUK_ASSERT(blen >= 1);

	if (data[0] == 0xffU) {
		return DUK_SYMBOL_TYPE_HIDDEN;
	} else if (data[0] == 0x82U) {
		return DUK_SYMBOL_TYPE_HIDDEN;
	} else if (data[0] == 0x80U) {
		return DUK_SYMBOL_TYPE_GLOBAL;
	} else if (data[blen - 1] != 0xffU) {
		return DUK_SYMBOL_TYPE_LOCAL;
	} else {
		return DUK_SYMBOL_TYPE_WELLKNOWN;
	}
}

DUK_LOCAL const char *duk__get_symbol_type_string(duk_hstring *h) {
	duk_small_uint_t idx;
	idx = duk__get_symbol_type(h);
	DUK_ASSERT(idx < sizeof(duk__symbol_type_strings));
	return duk__symbol_type_strings[idx];
}

DUK_INTERNAL void duk_push_objproto_tostring_hobject(duk_hthread *thr, duk_hobject *obj, duk_bool_t avoid_side_effects) {
	/* Just rely on the duk_tval variant for now. */
	duk_push_hobject(thr, obj);
	duk_push_objproto_tostring_tval(thr, DUK_GET_TVAL_NEGIDX(thr, -1), avoid_side_effects);
	duk_remove_m2(thr);
}

/* Push Object.prototype.toString() output for 'tv'.
 * https://tc39.es/ecma262/#sec-object.prototype.tostring
 *
 * This operation is almost error free but does throw for revoked Proxies.
 * When avoid_side_effects=1 we avoid a throw for a revoked Proxy.
 */
DUK_INTERNAL void duk_push_objproto_tostring_tval(duk_hthread *thr, duk_tval *tv, duk_bool_t avoid_side_effects) {
	duk_hobject *h;
	duk_hobject *h_resolved;
	duk_small_uint_t htype;
	duk_small_uint_t stridx;

	DUK_ASSERT_API_ENTRY(thr);
	DUK_ASSERT(tv != NULL); /* Unstable pointer. */

	/* Specification algorithm is roughly:
	 *
	 * - Handle undefined/null specially, ToObject() coerce rest.
	 *
	 * - Special case Arrays, which bypass @@toStringTag checks.
	 *   For the Array check, must follow Proxy chain to the final
	 *   non-Proxy value.
	 *
	 * - Choose a base builtinTag based on object internal slots.
	 *   For Proxies this is complex because it depends on what
	 *   internal slots are copied (e.g. [[Call]] is, but
	 *   [[BooleanData]] is not).  So for a proxied function the
	 *   builtinTag is 'Function' but for a proxied Boolean it is
	 *   'Object'.
	 *
	 * - Look up @@toStringTag, and if it is a string, override
	 *   the default builtinTag.  (This does not happen for Arrays.)
	 *
	 * - Return "[object <tag>]".
	 *
	 * We'd like to avoid the actual ToObject() conversion, but even
	 * for primitive types the prototype may have a @@toStringTag.
	 * What's worse, the @@toStringTag property may be a getter, and
	 * it must see the object coerced value.
	 *
	 * For now, do an actual object coercion.  This could be avoided by
	 * doing a side effect free lookup to see if a getter would be invoked.
	 * If not, the value can be read directly and the object coercion could
	 * be avoided.  This may not be worth it in practice, because
	 * Object.prototype.toString() is usually not performance critical.
	 *
	 * For Proxies, internal slot checks are made against the Proxy, not
	 * a resolved Proxy target.  ProxyCreate() only copies [[Call]] and
	 * [[Construct]] slots into the Proxy (https://tc39.es/ecma262/#sec-proxycreate).
	 * So Proxies behave as follows with respect to slot checks:
	 *
	 *   [[ParameterMap]]  => not copied, 'Object'
	 *   [[Call]]          => copied, 'Function'
	 *   [[ErrorData]]     => not copied, 'Object'
	 *   [[BooleanData]]   => not copied, 'Object'
	 *   [[NumberData]]    => not copied, 'Object'
	 *   [[StringData]]    => not copied, 'Object'
	 *   [[DateValue]]     => not copied, 'Object'
	 *   [[RegExpMatcher]] => not copied, 'Object'
	 */

	if (DUK_TVAL_IS_UNDEFINED(tv)) {
		stridx = DUK_STRIDX_UC_UNDEFINED;
		goto push_stridx;
	} else if (DUK_TVAL_IS_NULL(tv)) {
		stridx = DUK_STRIDX_UC_NULL;
		goto push_stridx;
	} else {
		DUK_ASSERT(!DUK_TVAL_IS_UNUSED(tv));

		duk_push_tval(thr, tv);
		h = duk_to_hobject(thr, -1);
		tv = NULL;
		DUK_ASSERT(h != NULL);

		/* Here we want to detect a revoked Proxy without throwing. */
		h_resolved = duk_hobject_resolve_proxy_target_nothrow(thr, h);
		if (h_resolved == NULL) {
			if (avoid_side_effects) {
				duk_push_string(thr, "RevokedProxy");
				goto tag_pushed;
			} else {
				DUK_ERROR_TYPE_PROXY_REVOKED(thr);
			}
		} else if (DUK_HOBJECT_IS_ARRAY(h_resolved)) {
			/* IsArray() resolves Proxy chain target recursively. */
			stridx = DUK_STRIDX_UC_ARRAY;
		} else {
			/* Use htype of the coerced object, with Proxy chain unresolved,
			 * as a base.  This reflects internal slots like [[ErrorData]] etc,
			 * except for [[Call]] for Proxies.
			 */
			htype = DUK_HEAPHDR_GET_HTYPE((duk_heaphdr *) h);
			stridx = duk_htype_to_stridx[htype];
			if (DUK_HOBJECT_HAS_CALLABLE(h)) {
				/* Needed for Proxies which may or may not be callable,
				 * so the result is not directly based on htype.
				 */
				stridx = DUK_STRIDX_UC_FUNCTION;
			}
			DUK_ASSERT(htype != DUK_HTYPE_ARRAY);

#if defined(DUK_USE_SYMBOL_BUILTIN)
			if (!avoid_side_effects) {
				/* XXX: better handling with avoid_side_effects == 1; lookup tval
				 * without Proxy or getter side effects, and use it in sanitized
				 * form if it's a string.
				 */
				(void) duk_prop_getvalue_stridx_push(thr,
				                                     duk_get_top_index_known(thr),
				                                     DUK_STRIDX_WELLKNOWN_SYMBOL_TO_STRING_TAG);
				if (duk_is_string_notsymbol(thr, -1)) {
					duk_remove_m2(thr); /* -> [ ... tag ] */
					goto tag_pushed;
				}
				duk_pop_known(thr); /* -> [ ... target ] */
			}
#else
			DUK_UNREF(avoid_side_effects);
#endif
		}
		duk_pop_known(thr); /* -> [ ... ] */
	}

push_stridx:
	duk_push_hstring_stridx(thr, stridx); /* -> [ ... tag ] */

#if defined(DUK_USE_SYMBOL_BUILTIN)
tag_pushed:
#endif
	/* [ ... string ] */
	duk_push_literal(thr, "[object "); /* -> [ ... tag "[object" ] */
	duk_insert(thr, -2);
	duk_push_literal(thr, "]");
	duk_concat(thr, 3); /* [ ... "[object" tag "]" ] -> [ ... res ] */
}

/*
 *  Push readable string summarizing a duk_tval or other argument type.  The
 *  operation is side effect free and will only throw from internal errors
 *  (e.g. out of memory).  This is used by e.g. property access code to
 *  summarize a key/base safely, and is not intended to be fast (but small
 *  and safe).
 */

/* String limits for summary strings. */
#define DUK__READABLE_SUMMARY_MAXCHARS 96 /* maximum supported by helper */
#define DUK__READABLE_STRING_MAXCHARS  32 /* for strings/symbols */
#define DUK__READABLE_ERRMSG_MAXCHARS  96 /* for error messages */

/* String sanitizer which escapes ASCII control characters and a few other
 * ASCII characters, passes Unicode as is, and replaces invalid UTF-8 with
 * question marks.  No errors are thrown for any input string, except in out
 * of memory situations.
 */

DUK_LOCAL void duk__push_readable_hstring_unicode(duk_hthread *thr, duk_hstring *h_input, duk_small_uint_t maxchars) {
	const duk_uint8_t *p, *p_start, *p_end;
	duk_uint8_t buf[DUK_UNICODE_MAX_XUTF8_LENGTH * DUK__READABLE_SUMMARY_MAXCHARS + 2 /*quotes*/ + 3 /*periods*/];
	duk_uint8_t *q;
	duk_ucodepoint_t cp;
	duk_small_uint_t nchars;

	DUK_CTX_ASSERT_VALID(thr);
	DUK_ASSERT(h_input != NULL);
	DUK_ASSERT(maxchars <= DUK__READABLE_SUMMARY_MAXCHARS);

	p_start = (const duk_uint8_t *) duk_hstring_get_data(h_input);
	p_end = p_start + duk_hstring_get_bytelen(h_input);
	p = p_start;
	q = buf;

	nchars = 0;
	*q++ = (duk_uint8_t) DUK_ASC_SINGLEQUOTE;
	for (;;) {
		if (p >= p_end) {
			break;
		}
		if (nchars == maxchars) {
			*q++ = (duk_uint8_t) DUK_ASC_PERIOD;
			*q++ = (duk_uint8_t) DUK_ASC_PERIOD;
			*q++ = (duk_uint8_t) DUK_ASC_PERIOD;
			break;
		}
		if (duk_unicode_decode_xutf8(thr, &p, p_start, p_end, &cp)) {
			if (cp < 0x20 || cp == 0x7f || cp == DUK_ASC_SINGLEQUOTE || cp == DUK_ASC_BACKSLASH) {
				DUK_ASSERT(DUK_UNICODE_MAX_XUTF8_LENGTH >= 4); /* estimate is valid */
				DUK_ASSERT((cp >> 4) <= 0x0f);
				*q++ = (duk_uint8_t) DUK_ASC_BACKSLASH;
				*q++ = (duk_uint8_t) DUK_ASC_LC_X;
				*q++ = (duk_uint8_t) duk_lc_digits[cp >> 4];
				*q++ = (duk_uint8_t) duk_lc_digits[cp & 0x0f];
			} else {
				q += duk_unicode_encode_xutf8(cp, q);
			}
		} else {
			p++; /* advance manually */
			*q++ = (duk_uint8_t) DUK_ASC_QUESTION;
		}
		nchars++;
	}
	*q++ = (duk_uint8_t) DUK_ASC_SINGLEQUOTE;

	duk_push_lstring(thr, (const char *) buf, (duk_size_t) (q - buf));
}

DUK_LOCAL void duk__push_readable_hobject(duk_hthread *thr, duk_hobject *obj, duk_bool_t error_aware) {
	if (error_aware &&
	    duk_hobject_prototype_chain_contains(thr, obj, thr->builtins[DUK_BIDX_ERROR_PROTOTYPE], 1 /*ignore_loop*/)) {
		/* Get error message in a side effect free way if
		 * possible; if not, summarize as a generic object.
		 * Error message currently gets quoted.
		 */
		duk_tval *tv_msg;
		tv_msg = duk_hobject_find_entry_tval_ptr_stridx(thr->heap, obj, DUK_STRIDX_MESSAGE);
		if (tv_msg != NULL && DUK_TVAL_IS_STRING(tv_msg)) {
			/* It's critical to avoid recursion so
			 * only summarize a string .message.
			 */
			duk__push_readable_hstring_unicode(thr, DUK_TVAL_GET_STRING(tv_msg), DUK__READABLE_ERRMSG_MAXCHARS);
			return;
		}
	}
	duk_push_objproto_tostring_hobject(thr, obj, 1 /*avoid_side_effects*/);
}

DUK_LOCAL const char *duk__push_readable_tval(duk_hthread *thr, duk_tval *tv, duk_bool_t error_aware) {
	DUK_CTX_ASSERT_VALID(thr);
	/* 'tv' may be NULL */

	if (tv == NULL) {
		duk_push_literal(thr, "none");
	} else {
		switch (DUK_TVAL_GET_TAG(tv)) {
		case DUK_TAG_STRING: {
			duk_hstring *h = DUK_TVAL_GET_STRING(tv);
			if (DUK_HSTRING_HAS_SYMBOL(h)) {
				/* XXX: string summary produces question marks
				 * so this is not very ideal.
				 */
				duk_push_literal(thr, "[Symbol ");
				duk_push_string(thr, duk__get_symbol_type_string(h));
				duk_push_literal(thr, " ");
				duk__push_readable_hstring_unicode(thr, h, DUK__READABLE_STRING_MAXCHARS);
				duk_push_literal(thr, "]");
				duk_concat(thr, 5);
				break;
			}
			duk__push_readable_hstring_unicode(thr, h, DUK__READABLE_STRING_MAXCHARS);
			break;
		}
		case DUK_TAG_OBJECT: {
			duk_hobject *h = DUK_TVAL_GET_OBJECT(tv);
			DUK_ASSERT(h != NULL);
			duk__push_readable_hobject(thr, h, error_aware);
			break;
		}
		case DUK_TAG_BUFFER: {
			/* While plain buffers mimic Uint8Arrays, they summarize differently.
			 * This is useful so that the summarized string accurately reflects the
			 * internal type which may matter for figuring out bugs etc.
			 */
			/* XXX: Hex encoded, length limited buffer summary here? */
			duk_hbuffer *h = DUK_TVAL_GET_BUFFER(tv);
			DUK_ASSERT(h != NULL);
			duk_push_sprintf(thr, "[buffer:%ld]", (long) DUK_HBUFFER_GET_SIZE(h));
			break;
		}
		case DUK_TAG_POINTER: {
			/* Surround with parentheses like in JX, ensures NULL pointer
			 * is distinguishable from null value ("(null)" vs "null").
			 */
			duk_push_tval(thr, tv);
			duk_push_sprintf(thr, "(%s)", duk_to_string(thr, -1));
			duk_remove_m2(thr);
			break;
		}
		default: {
			duk_push_tval(thr, tv);
			break;
		}
		}
	}

	return duk_to_string(thr, -1);
}
DUK_INTERNAL const char *duk_push_readable_tval(duk_hthread *thr, duk_tval *tv) {
	DUK_ASSERT_API_ENTRY(thr);
	return duk__push_readable_tval(thr, tv, 0 /*error_aware*/);
}

DUK_INTERNAL const char *duk_push_readable_idx(duk_hthread *thr, duk_idx_t idx) {
	DUK_ASSERT_API_ENTRY(thr);
	return duk_push_readable_tval(thr, duk_get_tval(thr, idx));
}

DUK_INTERNAL const char *duk_push_readable_tval_erroraware(duk_hthread *thr, duk_tval *tv) {
	DUK_ASSERT_API_ENTRY(thr);
	return duk__push_readable_tval(thr, tv, 1 /*error_aware*/);
}

DUK_INTERNAL const char *duk_push_readable_hobject(duk_hthread *thr, duk_hobject *h) {
	DUK_ASSERT_API_ENTRY(thr);
	duk__push_readable_hobject(thr, h, 0 /*error_aware*/);
	return duk_to_string(thr, -1);
}

DUK_INTERNAL const char *duk_push_readable_hstring(duk_hthread *thr, duk_hstring *h) {
	DUK_ASSERT_API_ENTRY(thr);
	duk__push_readable_hstring_unicode(thr, h, DUK__READABLE_STRING_MAXCHARS);
	return duk_to_string(thr, -1);
}
