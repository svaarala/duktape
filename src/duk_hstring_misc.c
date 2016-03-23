/*
 *  Misc support functions
 */

#include "duk_internal.h"

DUK_INTERNAL duk_ucodepoint_t duk_hstring_char_code_at_raw(duk_hthread *thr, duk_hstring *h, duk_uint_t pos) {
	duk_uint32_t boff;
	const duk_uint8_t *p, *p_start, *p_end;
	duk_ucodepoint_t cp;

	/* Caller must check character offset to be inside the string. */
	DUK_ASSERT(thr != NULL);
	DUK_ASSERT(h != NULL);
	DUK_ASSERT_DISABLE(pos >= 0);  /* unsigned */
	DUK_ASSERT(pos < (duk_uint_t) DUK_HSTRING_GET_CHARLEN(h));

	boff = duk_heap_strcache_offset_char2byte(thr, h, (duk_uint32_t) pos);
	DUK_DDD(DUK_DDDPRINT("charCodeAt: pos=%ld -> boff=%ld, str=%!O",
	                     (long) pos, (long) boff, (duk_heaphdr *) h));
	DUK_ASSERT_DISABLE(boff >= 0);
	DUK_ASSERT(boff < DUK_HSTRING_GET_BYTELEN(h));

	p_start = DUK_HSTRING_GET_DATA(h);
	p_end = p_start + DUK_HSTRING_GET_BYTELEN(h);
	p = p_start + boff;
	DUK_DDD(DUK_DDDPRINT("p_start=%p, p_end=%p, p=%p",
	                     (const void *) p_start, (const void *) p_end,
	                     (const void *) p));

	/* This may throw an error though not for valid E5 strings. */
	cp = duk_unicode_decode_xutf8_checked(thr, &p, p_start, p_end);
	return cp;
}

#if !defined(DUK_USE_HSTRING_CLEN)
DUK_INTERNAL duk_size_t duk_hstring_get_charlen(duk_hstring *h) {
	if (DUK_HSTRING_HAS_ASCII(h)) {
		/* Most practical strings will go here. */
		return DUK_HSTRING_GET_BYTELEN(h);
	} else {
		return duk_unicode_unvalidated_utf8_length(DUK_HSTRING_GET_DATA(h), DUK_HSTRING_GET_BYTELEN(h));
	}
}
#endif  /* !DUK_USE_HSTRING_CLEN */
