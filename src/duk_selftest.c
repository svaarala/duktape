/*
 *  Self tests to ensure execution environment is sane.  Intended to catch
 *  compiler/platform problems which cannot be detected at compile time.
 */

#include "duk_internal.h"

#if defined(DUK_USE_SELF_TESTS)

/*
 *  Self test for union aliasing, see misc/clang_aliasing.c.
 */

typedef union {
	double d;
	unsigned char c[8];
} duk_alias_test_union;

#define DUK_UNION_CMP()  do { \
		if (DUK_MEMCMP((void *) &a, (void *) &b, sizeof(a)) != 0) { \
			DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "union aliasing self test failed: compare failed"); \
		} \
	} while (0)

static void duk_selftest_tval_aliasing(void) {
	duk_alias_test_union a, b;

	if (sizeof(duk_alias_test_union) != 8) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "union aliasing self test failed: invalid union size");
	}

	/* Test signaling NaN and alias assignment in all
	 * endianness combinations.
	 */

	/* little endian */
	a.c[0] = 0x11; a.c[1] = 0x22; a.c[2] = 0x33; a.c[3] = 0x44;
	a.c[4] = 0x00; a.c[5] = 0x00; a.c[6] = 0xf1; a.c[7] = 0xff;
	b = a;
	DUK_UNION_CMP();

	/* big endian */
	a.c[0] = 0xff; a.c[1] = 0xf1; a.c[2] = 0x00; a.c[3] = 0x00;
	a.c[4] = 0x44; a.c[5] = 0x33; a.c[6] = 0x22; a.c[7] = 0x11;
	b = a;
	DUK_UNION_CMP();

	/* middle endian */
	a.c[0] = 0x00; a.c[1] = 0x00; a.c[2] = 0xf1; a.c[3] = 0xff;
	a.c[4] = 0x11; a.c[5] = 0x22; a.c[6] = 0x33; a.c[7] = 0x44;
	b = a;
	DUK_UNION_CMP();
}
#undef DUK_UNION_CMP

/*
 *  Self test main
 */

void duk_selftest_run_tests(void) {
	duk_selftest_tval_aliasing();
}

#endif  /* DUK_USE_SELF_TESTS */
