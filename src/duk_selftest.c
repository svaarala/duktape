/*
 *  Self tests to ensure execution environment is sane.  Intended to catch
 *  compiler/platform problems which cannot be detected at compile time.
 */

#include "duk_internal.h"

#if defined(DUK_USE_SELF_TESTS)

/*
 *  Test union for double-related self tests
 */

typedef union {
	double d;
	unsigned char c[8];
} duk_test_union;

#define DUK_UNION_CMP_TRUE(a,b)  do { \
		if (DUK_MEMCMP((void *) (a), (void *) (b), sizeof(duk_test_union)) != 0) { \
			DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: double union compares false (expected true)"); \
		} \
	} while (0)

#define DUK_UNION_CMP_FALSE(a,b)  do { \
		if (DUK_MEMCMP((void *) (a), (void *) (b), sizeof(duk_test_union)) == 0) { \
			DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: double union compares true (expected false)"); \
		} \
	} while (0)


/*
 *  Self test for basic double / byte union memory layout

 */
static void duk_selftest_union_size(void) {
	if (sizeof(duk_test_union) != 8) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: invalid union size");
	}
}

/*
 *  Self test for union aliasing, see misc/clang_aliasing.c.
 */

static void duk_selftest_double_aliasing(void) {
	duk_test_union a, b;

	/* Test signaling NaN and alias assignment in all
	 * endianness combinations.
	 */

	/* little endian */
	a.c[0] = 0x11; a.c[1] = 0x22; a.c[2] = 0x33; a.c[3] = 0x44;
	a.c[4] = 0x00; a.c[5] = 0x00; a.c[6] = 0xf1; a.c[7] = 0xff;
	b = a;
	DUK_UNION_CMP_TRUE(&a, &b);

	/* big endian */
	a.c[0] = 0xff; a.c[1] = 0xf1; a.c[2] = 0x00; a.c[3] = 0x00;
	a.c[4] = 0x44; a.c[5] = 0x33; a.c[6] = 0x22; a.c[7] = 0x11;
	b = a;
	DUK_UNION_CMP_TRUE(&a, &b);

	/* middle endian */
	a.c[0] = 0x00; a.c[1] = 0x00; a.c[2] = 0xf1; a.c[3] = 0xff;
	a.c[4] = 0x11; a.c[5] = 0x22; a.c[6] = 0x33; a.c[7] = 0x44;
	b = a;
	DUK_UNION_CMP_TRUE(&a, &b);
}

static void duk_selftest_double_zero_sign(void) {
	volatile duk_test_union a, b;

	a.d = 0.0;
	b.d = -a.d;
	DUK_UNION_CMP_FALSE(&a, &b);
}

/*
 *  Self test main
 */

void duk_selftest_run_tests(void) {
	duk_selftest_union_size();
	duk_selftest_double_aliasing();
	duk_selftest_double_zero_sign();
}

#undef DUK_UNION_CMP

#endif  /* DUK_USE_SELF_TESTS */
