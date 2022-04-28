/*
 *  Misc duk_hstring support functions.
 */

#include "duk_internal.h"

/*
 *  Simple getters and setters.
 */

DUK_INTERNAL duk_bool_t duk_hstring_is_ascii(duk_hstring *h) {
	return DUK_HSTRING_HAS_ASCII(h);
}

DUK_INTERNAL duk_bool_t duk_hstring_is_empty(duk_hstring *h) {
	return duk_hstring_get_bytelen(h) == 0U;
}

DUK_INTERNAL duk_uint32_t duk_hstring_get_hash(duk_hstring *h) {
#if defined(DUK_USE_STRHASH16)
	return (duk_uint32_t) h->hash;
#else
	return h->hash;
#endif
}

DUK_INTERNAL void duk_hstring_set_hash(duk_hstring *h, duk_uint32_t hash) {
#if defined(DUK_USE_STRHASH16)
	DUK_ASSERT(hash <= 0xffffUL);
	h->hash = hash;
#else
	h->hash = hash;
#endif
}

#if defined(DUK_USE_HSTRING_EXTDATA)
DUK_INTERNAL const duk_uint8_t *duk_hstring_get_extdata(duk_hstring *h) {
	DUK_ASSERT(DUK_HSTRING_HAS_EXTDATA(h));
	return h->extdata;
}
#endif

DUK_INTERNAL const duk_uint8_t *duk_hstring_get_data(duk_hstring *h) {
#if defined(DUK_USE_HSTRING_EXTDATA)
	if (DUK_HSTRING_HAS_EXTDATA(h)) {
		return duk_hstring_get_extdata(h);
	} else {
		return (const duk_uint8_t *) (x + 1);
	}
#else
	return (const duk_uint8_t *) (h + 1);
#endif
}

DUK_INTERNAL const duk_uint8_t *duk_hstring_get_data_and_bytelen(duk_hstring *h, duk_size_t *out_blen) {
	DUK_ASSERT(out_blen != NULL);
	*out_blen = duk_hstring_get_bytelen(h);
	return duk_hstring_get_data(h);
}

DUK_INTERNAL const duk_uint8_t *duk_hstring_get_data_end(duk_hstring *h) {
	return duk_hstring_get_data(h) + duk_hstring_get_bytelen(h);
}

/*
 *  duk_hstring charlen
 */

DUK_INTERNAL void duk_hstring_set_charlen(duk_hstring *h, duk_size_t len) {
#if defined(DUK_USE_HSTRING_CLEN)
#if defined(DUK_USE_STRLEN16)
	DUK_ASSERT(len <= 0xffffUL);
	h->clen16 = len;
#else
	DUK_ASSERT(len <= 0xffffffffUL);
	h->clen = len;
#endif
#else
	DUK_UNREF(len);
#endif
}

/*
 *  duk_hstring charlen, when lazy charlen enabled.
 */

DUK_LOCAL duk_size_t duk__hstring_get_charlen_slowpath(duk_hstring *h) {
	if (DUK_LIKELY(DUK_HSTRING_HAS_ASCII(h))) {
		/* Most practical strings will go here. */
		DUK_ASSERT(!DUK_HSTRING_HAS_SYMBOL(h));
		return duk_hstring_get_bytelen(h);
	} else {
		duk_size_t res;

		/* XXX: here we could use the strcache to speed up the
		 * computation (matters for 'i < str.length' loops).
		 */

		if (DUK_HSTRING_HAS_SYMBOL(h)) {
			return 0;
		}
		res = duk_unicode_wtf8_charlength(duk_hstring_get_data(h), duk_hstring_get_bytelen(h));

		return res;
	}
}

#if defined(DUK_USE_HSTRING_CLEN)
DUK_INTERNAL DUK_HOT duk_size_t duk_hstring_get_charlen(duk_hstring *h) {
#if defined(DUK_USE_STRLEN16)
	return h->clen16;
#else
	return h->clen;
#endif
}
#else /* DUK_USE_HSTRING_CLEN */
DUK_INTERNAL DUK_HOT duk_size_t duk_hstring_get_charlen(duk_hstring *h) {
	/* Always use slow path. */
	return duk__hstring_get_charlen_slowpath(h);
}
#endif /* DUK_USE_HSTRING_CLEN */

/*
 *  duk_hstring charCodeAt, with and without surrogate awareness.
 */

DUK_INTERNAL duk_ucodepoint_t duk_hstring_char_code_at_raw(duk_hthread *thr,
                                                           duk_hstring *h,
                                                           duk_uint_t pos,
                                                           duk_bool_t surrogate_aware) {
	return duk_unicode_wtf8_charcodeat_helper(thr, h, pos, surrogate_aware);
}

/*
 *  Bytelen.
 */

DUK_INTERNAL DUK_HOT duk_size_t duk_hstring_get_bytelen(duk_hstring *h) {
#if defined(DUK_USE_STRLEN16)
	return h->hdr.h_strextra16;
#else
	return h->blen;
#endif
}

DUK_INTERNAL void duk_hstring_set_bytelen(duk_hstring *h, duk_size_t len) {
#if defined(DUK_USE_STRLEN16)
	DUK_ASSERT(len <= 0xffffUL);
	h->hdr.h_strextra16 = len;
#else
	DUK_ASSERT(len <= 0xffffffffUL);
	h->blen = len;
#endif
}

/*
 *  Arridx.
 */

DUK_INTERNAL duk_uarridx_t duk_hstring_get_arridx_fast(duk_hstring *h) {
#if defined(DUK_USE_HSTRING_ARRIDX)
	return h->arridx;
#else
	/* Get array index related to string (or return DUK_ARRIDX_NONE);
	 * avoids helper call if string has no array index value.
	 */
	if (DUK_HSTRING_HAS_ARRIDX(h)) {
		return duk_js_to_arrayindex_hstring_fast_known(h);
	} else {
		return DUK_ARRIDX_NONE;
	}
#endif
}

DUK_INTERNAL duk_uarridx_t duk_hstring_get_arridx_fast_known(duk_hstring *h) {
	DUK_ASSERT(DUK_HSTRING_HAS_ARRIDX(h));

#if defined(DUK_USE_HSTRING_ARRIDX)
	return h->arridx;
#else
	return duk_js_to_arrayindex_hstring_fast_known(h);
#endif
}

DUK_INTERNAL duk_uarridx_t duk_hstring_get_arridx_slow(duk_hstring *h) {
#if defined(DUK_USE_HSTRING_ARRIDX)
	return h->arridx;
#else
	return duk_js_to_arrayindex_hstring_fast(h);
#endif
}

/*
 *  Compare duk_hstring to an ASCII cstring.
 */

DUK_INTERNAL duk_bool_t duk_hstring_equals_ascii_cstring(duk_hstring *h, const char *cstr) {
	duk_size_t len;

	DUK_ASSERT(h != NULL);
	DUK_ASSERT(cstr != NULL);

	len = DUK_STRLEN(cstr);
	if (len != duk_hstring_get_bytelen(h)) {
		return 0;
	}
	if (duk_memcmp((const void *) cstr, (const void *) duk_hstring_get_data(h), len) == 0) {
		return 1;
	}
	return 0;
}

DUK_INTERNAL duk_bool_t duk_hstring_is_symbol_initial_byte(duk_uint8_t t) {
	return (t >= 0x80) && (t <= 0x82U || t == 0xffU);
}

DUK_INTERNAL duk_bool_t duk_hstring_is_valid_hstring_data(const duk_uint8_t *p, duk_size_t blen) {
	DUK_ASSERT(p != NULL || blen == 0);
	if (blen > 0) {
		duk_uint8_t ib = p[0];
		if (duk_hstring_is_symbol_initial_byte(ib)) {
			/* Should validate Symbol, no validation now. */
			return 1;
		} else {
			return duk_unicode_is_valid_wtf8(p, blen);
		}
	}
	return 1;
}
