/*
 *  duk_hstring assertion helpers.
 */

#include "duk_internal.h"

#if defined(DUK_USE_ASSERTIONS)

DUK_INTERNAL void duk_hstring_assert_valid(duk_hstring *h) {
	DUK_ASSERT(h != NULL);

	if (DUK_HSTRING_HAS_SYMBOL(h)) {
		/* XXX: add checks here. */
	} else {
		const duk_uint8_t *str = duk_hstring_get_data(h);
		duk_size_t blen = duk_hstring_get_bytelen(h);

		DUK_ASSERT(duk_unicode_is_valid_wtf8(str, blen));
	}
}

#endif /* DUK_USE_ASSERTIONS */
