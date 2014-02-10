/*
 *  Self tests to ensure execution environment is sane.  Intended to catch
 *  compiler/platform problems which cannot be detected at compile time.
 */

#include "duk_internal.h"

#if defined(DUK_USE_SELF_TESTS)

/*
 *  Unions and structs for self tests
 */

typedef union {
	double d;
	duk_uint8_t c[8];
} duk__test_double_union;

#define DUK__DBLUNION_CMP_TRUE(a,b)  do { \
		if (DUK_MEMCMP((void *) (a), (void *) (b), sizeof(duk__test_double_union)) != 0) { \
			DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: double union compares false (expected true)"); \
		} \
	} while (0)

#define DUK__DBLUNION_CMP_FALSE(a,b)  do { \
		if (DUK_MEMCMP((void *) (a), (void *) (b), sizeof(duk__test_double_union)) == 0) { \
			DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: double union compares true (expected false)"); \
		} \
	} while (0)

typedef union {
	duk_uint32_t i;
	duk_uint8_t c[8];
} duk__test_u32_union;

/*
 *  Two's complement arithmetic.
 */

static void duk__selftest_twos_complement(void) {
	volatile int test;
	test = -1;
	if (((duk_uint8_t *) &test)[0] != (duk_uint8_t) 0xff) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: two's complement arithmetic");
	}
}

/*
 *  Byte order.  Important to self check, because on some exotic platforms
 *  there is no actual detection but rather assumption based on platform
 *  defines.
 */

static void duk__selftest_byte_order(void) {
	duk__test_u32_union u1;
	duk__test_double_union u2;

	/*
	 *  >>> struct.pack('>d', 102030405060).encode('hex')
	 *  '4237c17c6dc40000'
	 */
#if defined(DUK_USE_LITTLE_ENDIAN)
	u1.c[0] = 0xef; u1.c[1] = 0xbe; u1.c[2] = 0xad; u1.c[3] = 0xde;
	u2.c[0] = 0x00; u2.c[1] = 0x00; u2.c[2] = 0xc4; u2.c[3] = 0x6d;
	u2.c[4] = 0x7c; u2.c[5] = 0xc1; u2.c[6] = 0x37; u2.c[7] = 0x42;
#elif defined(DUK_USE_MIDDLE_ENDIAN)
	u1.c[0] = 0xef; u1.c[1] = 0xbe; u1.c[2] = 0xad; u1.c[3] = 0xde;
	u2.c[0] = 0x7c; u2.c[1] = 0xc1; u2.c[2] = 0x37; u2.c[3] = 0x42;
	u2.c[4] = 0x00; u2.c[5] = 0x00; u2.c[6] = 0xc4; u2.c[7] = 0x6d;
#elif defined(DUK_USE_BIG_ENDIAN)
	u1.c[0] = 0xde; u1.c[1] = 0xad; u1.c[2] = 0xbe; u1.c[3] = 0xef;
	u2.c[0] = 0x42; u2.c[1] = 0x37; u2.c[2] = 0xc1; u2.c[3] = 0x7c;
	u2.c[4] = 0x6d; u2.c[5] = 0xc4; u2.c[6] = 0x00; u2.c[7] = 0x00;
#else
#error unknown endianness
#endif

	if (u1.i != (duk_uint32_t) 0xdeadbeefUL) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: duk_uint32_t byte order");
	}

	if (u2.d != (double) 102030405060.0) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: double byte order");
	}
}

/*
 *  Basic double / byte union memory layout.
 */

static void duk__selftest_double_union_size(void) {
	if (sizeof(duk__test_double_union) != 8) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: invalid union size");
	}
}

/*
 *  Union aliasing, see misc/clang_aliasing.c.
 */

static void duk__selftest_double_aliasing(void) {
	duk__test_double_union a, b;

	/* Test signaling NaN and alias assignment in all
	 * endianness combinations.
	 */

	/* little endian */
	a.c[0] = 0x11; a.c[1] = 0x22; a.c[2] = 0x33; a.c[3] = 0x44;
	a.c[4] = 0x00; a.c[5] = 0x00; a.c[6] = 0xf1; a.c[7] = 0xff;
	b = a;
	DUK__DBLUNION_CMP_TRUE(&a, &b);

	/* big endian */
	a.c[0] = 0xff; a.c[1] = 0xf1; a.c[2] = 0x00; a.c[3] = 0x00;
	a.c[4] = 0x44; a.c[5] = 0x33; a.c[6] = 0x22; a.c[7] = 0x11;
	b = a;
	DUK__DBLUNION_CMP_TRUE(&a, &b);

	/* middle endian */
	a.c[0] = 0x00; a.c[1] = 0x00; a.c[2] = 0xf1; a.c[3] = 0xff;
	a.c[4] = 0x11; a.c[5] = 0x22; a.c[6] = 0x33; a.c[7] = 0x44;
	b = a;
	DUK__DBLUNION_CMP_TRUE(&a, &b);
}

/*
 *  Zero sign, see misc/tcc_zerosign2.c.
 */

static void duk__selftest_double_zero_sign(void) {
	volatile duk__test_double_union a, b;

	a.d = 0.0;
	b.d = -a.d;
	DUK__DBLUNION_CMP_FALSE(&a, &b);
}

/*
 *  Self test main
 */

void duk_selftest_run_tests(void) {
	duk__selftest_twos_complement();
	duk__selftest_byte_order();
	duk__selftest_double_union_size();
	duk__selftest_double_aliasing();
	duk__selftest_double_zero_sign();
}

#undef DUK__DBLUNION_CMP_TRUE
#undef DUK__DBLUNION_CMP_FALSE

#endif  /* DUK_USE_SELF_TESTS */
