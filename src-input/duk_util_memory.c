/*
 *  Memcpy() etc.
 */

#include "duk_internal.h"

DUK_INTERNAL void duk_memcpy(void *dst, const void *src, duk_size_t len) {
#if !defined(DUK_USE_ALLOW_UNDEFINED_BEHAVIOR)
	/* For portability reasons both 'src' and 'dst' must be valid and
	 * non-NULL even if len is zero.  Check for that explicitly to avoid
	 * depending on platform specific behavior.  For the vast majority of
	 * actual targets a NULL pointer with a zero length is fine.
	 */
	if (DUK_UNLIKELY(len == 0U)) {
		return;
	}
#endif

	(void) DUK_MEMCPY(dst, src, len);
}

DUK_INTERNAL duk_small_int_t duk_memcmp(const void *s1, const void *s2, duk_size_t len) {
#if !defined(DUK_USE_ALLOW_UNDEFINED_BEHAVIOR)
	if (DUK_UNLIKELY(len == 0U)) {
		return 0;
	}
#endif

	return (duk_small_int_t) DUK_MEMCMP(s1, s2, len);
}
