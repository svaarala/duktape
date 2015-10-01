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
		if (DUK_MEMCMP((const void *) (a), (const void *) (b), sizeof(duk__test_double_union)) != 0) { \
			DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: double union compares false (expected true)"); \
		} \
	} while (0)

#define DUK__DBLUNION_CMP_FALSE(a,b)  do { \
		if (DUK_MEMCMP((const void *) (a), (const void *) (b), sizeof(duk__test_double_union)) == 0) { \
			DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: double union compares true (expected false)"); \
		} \
	} while (0)

typedef union {
	duk_uint32_t i;
	duk_uint8_t c[8];
} duk__test_u32_union;

/*
 *  Various sanity checks for typing
 */

DUK_LOCAL void duk__selftest_types(void) {
	if (!(sizeof(duk_int8_t) == 1 &&
	      sizeof(duk_uint8_t) == 1 &&
	      sizeof(duk_int16_t) == 2 &&
	      sizeof(duk_uint16_t) == 2 &&
	      sizeof(duk_int32_t) == 4 &&
	      sizeof(duk_uint32_t) == 4)) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: duk_(u)int{8,16,32}_t size");
	}
#if defined(DUK_USE_64BIT_OPS)
	if (!(sizeof(duk_int64_t) == 8 &&
	      sizeof(duk_uint64_t) == 8)) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: duk_(u)int64_t size");
	}
#endif

	if (!(sizeof(duk_size_t) >= sizeof(duk_uint_t))) {
		/* Some internal code now assumes that all duk_uint_t values
		 * can be expressed with a duk_size_t.
		 */
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: duk_size_t is smaller than duk_uint_t");
	}
	if (!(sizeof(duk_int_t) >= 4)) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: duk_int_t is not 32 bits");
	}
}

/*
 *  Packed tval sanity
 */

DUK_LOCAL void duk__selftest_packed_tval(void) {
#if defined(DUK_USE_PACKED_TVAL)
	if (sizeof(void *) > 4) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: packed duk_tval in use but sizeof(void *) > 4");
	}
#endif
}

/*
 *  Two's complement arithmetic.
 */

DUK_LOCAL void duk__selftest_twos_complement(void) {
	volatile int test;
	test = -1;

	/* Note that byte order doesn't affect this test: all bytes in
	 * 'test' will be 0xFF for two's complement.
	 */
	if (((volatile duk_uint8_t *) &test)[0] != (duk_uint8_t) 0xff) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: two's complement arithmetic");
	}
}

/*
 *  Byte order.  Important to self check, because on some exotic platforms
 *  there is no actual detection but rather assumption based on platform
 *  defines.
 */

DUK_LOCAL void duk__selftest_byte_order(void) {
	duk__test_u32_union u1;
	duk__test_double_union u2;

	/*
	 *  >>> struct.pack('>d', 102030405060).encode('hex')
	 *  '4237c17c6dc40000'
	 */
#if defined(DUK_USE_INTEGER_LE)
	u1.c[0] = 0xef; u1.c[1] = 0xbe; u1.c[2] = 0xad; u1.c[3] = 0xde;
#elif defined(DUK_USE_INTEGER_ME)
#error integer mixed endian not supported now
#elif defined(DUK_USE_INTEGER_BE)
	u1.c[0] = 0xde; u1.c[1] = 0xad; u1.c[2] = 0xbe; u1.c[3] = 0xef;
#else
#error unknown integer endianness
#endif

#if defined(DUK_USE_DOUBLE_LE)
	u2.c[0] = 0x00; u2.c[1] = 0x00; u2.c[2] = 0xc4; u2.c[3] = 0x6d;
	u2.c[4] = 0x7c; u2.c[5] = 0xc1; u2.c[6] = 0x37; u2.c[7] = 0x42;
#elif defined(DUK_USE_DOUBLE_ME)
	u2.c[0] = 0x7c; u2.c[1] = 0xc1; u2.c[2] = 0x37; u2.c[3] = 0x42;
	u2.c[4] = 0x00; u2.c[5] = 0x00; u2.c[6] = 0xc4; u2.c[7] = 0x6d;
#elif defined(DUK_USE_DOUBLE_BE)
	u2.c[0] = 0x42; u2.c[1] = 0x37; u2.c[2] = 0xc1; u2.c[3] = 0x7c;
	u2.c[4] = 0x6d; u2.c[5] = 0xc4; u2.c[6] = 0x00; u2.c[7] = 0x00;
#else
#error unknown double endianness
#endif

	if (u1.i != (duk_uint32_t) 0xdeadbeefUL) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: duk_uint32_t byte order");
	}

	if (u2.d != (double) 102030405060.0) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: double byte order");
	}
}

/*
 *  DUK_BSWAP macros
 */

DUK_LOCAL void duk__selftest_bswap_macros(void) {
	duk_uint32_t x32;
	duk_uint16_t x16;
	duk_double_union du;
	duk_double_t du_diff;

	x16 = 0xbeefUL;
	x16 = DUK_BSWAP16(x16);
	if (x16 != (duk_uint16_t) 0xefbeUL) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: DUK_BSWAP16");
	}

	x32 = 0xdeadbeefUL;
	x32 = DUK_BSWAP32(x32);
	if (x32 != (duk_uint32_t) 0xefbeaddeUL) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: DUK_BSWAP32");
	}

	/* >>> struct.unpack('>d', '4000112233445566'.decode('hex'))
	 * (2.008366013071895,)
	 */

	du.uc[0] = 0x40; du.uc[1] = 0x00; du.uc[2] = 0x11; du.uc[3] = 0x22;
	du.uc[4] = 0x33; du.uc[5] = 0x44; du.uc[6] = 0x55; du.uc[7] = 0x66;
	DUK_DBLUNION_DOUBLE_NTOH(&du);
	du_diff = du.d - 2.008366013071895;
#if 0
	DUK_FPRINTF(DUK_STDERR, "du_diff: %lg\n", (double) du_diff);
#endif
	if (du_diff > 1e-15) {
		/* Allow very small lenience because some compilers won't parse
		 * exact IEEE double constants (happened in matrix testing with
		 * Linux gcc-4.8 -m32 at least).
		 */
#if 0
		DUK_FPRINTF(DUK_STDERR, "Result of DUK_DBLUNION_DOUBLE_NTOH: %02x %02x %02x %02x %02x %02x %02x %02x\n",
		            (unsigned int) du.uc[0], (unsigned int) du.uc[1],
		            (unsigned int) du.uc[2], (unsigned int) du.uc[3],
		            (unsigned int) du.uc[4], (unsigned int) du.uc[5],
		            (unsigned int) du.uc[6], (unsigned int) du.uc[7]);
#endif
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: DUK_DBLUNION_DOUBLE_NTOH");
	}
}

/*
 *  Basic double / byte union memory layout.
 */

DUK_LOCAL void duk__selftest_double_union_size(void) {
	if (sizeof(duk__test_double_union) != 8) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: invalid union size");
	}
}

/*
 *  Union aliasing, see misc/clang_aliasing.c.
 */

DUK_LOCAL void duk__selftest_double_aliasing(void) {
	duk__test_double_union a, b;

	/* This testcase fails when Emscripten-generated code runs on Firefox.
	 * It's not an issue because the failure should only affect packed
	 * duk_tval representation, which is not used with Emscripten.
	 */
#if !defined(DUK_USE_PACKED_TVAL)
	DUK_D(DUK_DPRINT("skip double aliasing self test when duk_tval is not packed"));
	return;
#endif

	/* Test signaling NaN and alias assignment in all endianness combinations.
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

	/* mixed endian */
	a.c[0] = 0x00; a.c[1] = 0x00; a.c[2] = 0xf1; a.c[3] = 0xff;
	a.c[4] = 0x11; a.c[5] = 0x22; a.c[6] = 0x33; a.c[7] = 0x44;
	b = a;
	DUK__DBLUNION_CMP_TRUE(&a, &b);
}

/*
 *  Zero sign, see misc/tcc_zerosign2.c.
 */

DUK_LOCAL void duk__selftest_double_zero_sign(void) {
	duk__test_double_union a, b;

	a.d = 0.0;
	b.d = -a.d;
	DUK__DBLUNION_CMP_FALSE(&a, &b);
}

/*
 *  Struct size/alignment if platform requires it
 *
 *  There are some compiler specific struct padding pragmas etc in use, this
 *  selftest ensures they're correctly detected and used.
 */

DUK_LOCAL void duk__selftest_struct_align(void) {
#if (DUK_USE_ALIGN_BY == 4)
	if ((sizeof(duk_hbuffer_fixed) % 4) != 0) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: sizeof(duk_hbuffer_fixed) not aligned to 4");
	}
#elif (DUK_USE_ALIGN_BY == 8)
	if ((sizeof(duk_hbuffer_fixed) % 8) != 0) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: sizeof(duk_hbuffer_fixed) not aligned to 8");
	}
#elif (DUK_USE_ALIGN_BY == 1)
	/* no check */
#else
#error invalid DUK_USE_ALIGN_BY
#endif
}

/*
 *  64-bit arithmetic
 *
 *  There are some platforms/compilers where 64-bit types are available
 *  but don't work correctly.  Test for known cases.
 */

DUK_LOCAL void duk__selftest_64bit_arithmetic(void) {
#if defined(DUK_USE_64BIT_OPS)
	volatile duk_int64_t i;
	volatile duk_double_t d;

	/* Catch a double-to-int64 cast issue encountered in practice. */
	d = 2147483648.0;
	i = (duk_int64_t) d;
	if (i != 0x80000000LL) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: casting 2147483648.0 to duk_int64_t failed");
	}
#else
	/* nop */
#endif
}

/*
 *  Casting
 */

DUK_LOCAL void duk__selftest_cast_double_to_small_uint(void) {
	/*
	 *  https://github.com/svaarala/duktape/issues/127#issuecomment-77863473
	 */

	duk_double_t d1, d2;
	duk_small_uint_t u;

	duk_double_t d1v, d2v;
	duk_small_uint_t uv;

	/* Test without volatiles */

	d1 = 1.0;
	u = (duk_small_uint_t) d1;
	d2 = (duk_double_t) u;

	if (!(d1 == 1.0 && u == 1 && d2 == 1.0 && d1 == d2)) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: double to duk_small_uint_t cast failed");
	}

	/* Same test with volatiles */

	d1v = 1.0;
	uv = (duk_small_uint_t) d1v;
	d2v = (duk_double_t) uv;

	if (!(d1v == 1.0 && uv == 1 && d2v == 1.0 && d1v == d2v)) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: double to duk_small_uint_t cast failed");
	}
}

DUK_LOCAL void duk__selftest_cast_double_to_uint32(void) {
	/*
	 *  This test fails on an exotic ARM target; double-to-uint
	 *  cast is incorrectly clamped to -signed- int highest value.
	 *
	 *  https://github.com/svaarala/duktape/issues/336
	 */

	duk_double_t dv;
	duk_uint32_t uv;

	dv = 3735928559.0;  /* 0xdeadbeef in decimal */
	uv = (duk_uint32_t) dv;

	if (uv != 0xdeadbeefUL) {
		DUK_PANIC(DUK_ERR_INTERNAL_ERROR, "self test failed: double to duk_uint32_t cast failed");
	}
}

/*
 *  Self test main
 */

DUK_INTERNAL void duk_selftest_run_tests(void) {
	duk__selftest_types();
	duk__selftest_packed_tval();
	duk__selftest_twos_complement();
	duk__selftest_byte_order();
	duk__selftest_bswap_macros();
	duk__selftest_double_union_size();
	duk__selftest_double_aliasing();
	duk__selftest_double_zero_sign();
	duk__selftest_struct_align();
	duk__selftest_64bit_arithmetic();
	duk__selftest_cast_double_to_small_uint();
	duk__selftest_cast_double_to_uint32();
}

#undef DUK__DBLUNION_CMP_TRUE
#undef DUK__DBLUNION_CMP_FALSE

#endif  /* DUK_USE_SELF_TESTS */
