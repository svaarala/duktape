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
	DUK_ASSERT(src != NULL);
	DUK_ASSERT(dst != NULL);
#else
	DUK_ASSERT(src != NULL || len == 0U);
	DUK_ASSERT(dst != NULL || len == 0U);
#endif

	(void) DUK_MEMCPY(dst, src, (size_t) len);
}

DUK_INTERNAL void duk_memmove(void *dst, const void *src, duk_size_t len) {
#if !defined(DUK_USE_ALLOW_UNDEFINED_BEHAVIOR)
	if (DUK_UNLIKELY(len == 0U)) {
		return;
	}
	DUK_ASSERT(src != NULL);
	DUK_ASSERT(dst != NULL);
#else
	DUK_ASSERT(src != NULL || len == 0U);
	DUK_ASSERT(dst != NULL || len == 0U);
#endif

	(void) DUK_MEMMOVE(dst, src, (size_t) len);
}

DUK_INTERNAL duk_small_int_t duk_memcmp(const void *s1, const void *s2, duk_size_t len) {
	duk_small_int_t ret;

#if !defined(DUK_USE_ALLOW_UNDEFINED_BEHAVIOR)
	if (DUK_UNLIKELY(len == 0U)) {
		return 0;
	}
	DUK_ASSERT(s1 != NULL);
	DUK_ASSERT(s2 != NULL);
#else
	DUK_ASSERT(s1 != NULL || len == 0U);
	DUK_ASSERT(s2 != NULL || len == 0U);
#endif

	ret = (duk_small_int_t) DUK_MEMCMP(s1, s2, (size_t) len);
	DUK_ASSERT(ret == 0 || len > 0);  /* If len == 0, must compare equal. */
	return ret;
}

DUK_INTERNAL void duk_memset(void *s, duk_small_int_t c, duk_size_t len) {
	(void) DUK_MEMSET(s, (int) c, (size_t) len);
}

DUK_INTERNAL void duk_memzero(void *s, duk_size_t len) {
	(void) DUK_MEMZERO(s, (size_t) len);
}
