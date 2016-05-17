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

/* Self test failed.  Expects a local variable 'error_count' to exist. */
#define DUK__FAILED(msg)  do { \
		DUK_D(DUK_DPRINT("self test failed: " #msg)); \
		error_count++; \
	} while (0)

#define DUK__DBLUNION_CMP_TRUE(a,b)  do { \
		if (DUK_MEMCMP((const void *) (a), (const void *) (b), sizeof(duk__test_double_union)) != 0) { \
			DUK__FAILED("double union compares false (expected true)"); \
		} \
	} while (0)

#define DUK__DBLUNION_CMP_FALSE(a,b)  do { \
		if (DUK_MEMCMP((const void *) (a), (const void *) (b), sizeof(duk__test_double_union)) == 0) { \
			DUK__FAILED("double union compares true (expected false)"); \
		} \
	} while (0)

typedef union {
	duk_uint32_t i;
	duk_uint8_t c[8];
} duk__test_u32_union;

/*
 *  Various sanity checks for typing
 */

DUK_LOCAL duk_uint_t duk__selftest_types(void) {
	duk_uint_t error_count = 0;

	if (!(sizeof(duk_int8_t) == 1 &&
	      sizeof(duk_uint8_t) == 1 &&
	      sizeof(duk_int16_t) == 2 &&
	      sizeof(duk_uint16_t) == 2 &&
	      sizeof(duk_int32_t) == 4 &&
	      sizeof(duk_uint32_t) == 4)) {
		DUK__FAILED("duk_(u)int{8,16,32}_t size");
	}
#if defined(DUK_USE_64BIT_OPS)
	if (!(sizeof(duk_int64_t) == 8 &&
	      sizeof(duk_uint64_t) == 8)) {
		DUK__FAILED("duk_(u)int64_t size");
	}
#endif

	if (!(sizeof(duk_size_t) >= sizeof(duk_uint_t))) {
		/* Some internal code now assumes that all duk_uint_t values
		 * can be expressed with a duk_size_t.
		 */
		DUK__FAILED("duk_size_t is smaller than duk_uint_t");
	}
	if (!(sizeof(duk_int_t) >= 4)) {
		DUK__FAILED("duk_int_t is not 32 bits");
	}

	return error_count;
}

/*
 *  Packed tval sanity
 */

DUK_LOCAL duk_uint_t duk__selftest_packed_tval(void) {
	duk_uint_t error_count = 0;

#if defined(DUK_USE_PACKED_TVAL)
	if (sizeof(void *) > 4) {
		DUK__FAILED("packed duk_tval in use but sizeof(void *) > 4");
	}
#endif

	return error_count;
}

/*
 *  Two's complement arithmetic.
 */

DUK_LOCAL duk_uint_t duk__selftest_twos_complement(void) {
	duk_uint_t error_count = 0;
	volatile int test;
	test = -1;

	/* Note that byte order doesn't affect this test: all bytes in
	 * 'test' will be 0xFF for two's complement.
	 */
	if (((volatile duk_uint8_t *) &test)[0] != (duk_uint8_t) 0xff) {
		DUK__FAILED("two's complement arithmetic");
	}

	return error_count;
}

/*
 *  Byte order.  Important to self check, because on some exotic platforms
 *  there is no actual detection but rather assumption based on platform
 *  defines.
 */

DUK_LOCAL duk_uint_t duk__selftest_byte_order(void) {
	duk_uint_t error_count = 0;
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
		DUK__FAILED("duk_uint32_t byte order");
	}

	if (u2.d != (double) 102030405060.0) {
		DUK__FAILED("double byte order");
	}

	return error_count;
}

/*
 *  DUK_BSWAP macros
 */

DUK_LOCAL duk_uint_t duk__selftest_bswap_macros(void) {
	duk_uint_t error_count = 0;
	duk_uint32_t x32;
	duk_uint16_t x16;
	duk_double_union du;
	duk_double_t du_diff;

	x16 = 0xbeefUL;
	x16 = DUK_BSWAP16(x16);
	if (x16 != (duk_uint16_t) 0xefbeUL) {
		DUK__FAILED("DUK_BSWAP16");
	}

	x32 = 0xdeadbeefUL;
	x32 = DUK_BSWAP32(x32);
	if (x32 != (duk_uint32_t) 0xefbeaddeUL) {
		DUK__FAILED("DUK_BSWAP32");
	}

	/* >>> struct.unpack('>d', '4000112233445566'.decode('hex'))
	 * (2.008366013071895,)
	 */

	du.uc[0] = 0x40; du.uc[1] = 0x00; du.uc[2] = 0x11; du.uc[3] = 0x22;
	du.uc[4] = 0x33; du.uc[5] = 0x44; du.uc[6] = 0x55; du.uc[7] = 0x66;
	DUK_DBLUNION_DOUBLE_NTOH(&du);
	du_diff = du.d - 2.008366013071895;
#if 0
	DUK_D(DUK_DPRINT("du_diff: %lg\n", (double) du_diff));
#endif
	if (du_diff > 1e-15) {
		/* Allow very small lenience because some compilers won't parse
		 * exact IEEE double constants (happened in matrix testing with
		 * Linux gcc-4.8 -m32 at least).
		 */
#if 0
		DUK_D(DUK_DPRINT("Result of DUK_DBLUNION_DOUBLE_NTOH: %02x %02x %02x %02x %02x %02x %02x %02x\n",
		            (unsigned int) du.uc[0], (unsigned int) du.uc[1],
		            (unsigned int) du.uc[2], (unsigned int) du.uc[3],
		            (unsigned int) du.uc[4], (unsigned int) du.uc[5],
		            (unsigned int) du.uc[6], (unsigned int) du.uc[7]));
#endif
		DUK__FAILED("DUK_DBLUNION_DOUBLE_NTOH");
	}

	return error_count;
}

/*
 *  Basic double / byte union memory layout.
 */

DUK_LOCAL duk_uint_t duk__selftest_double_union_size(void) {
	duk_uint_t error_count = 0;

	if (sizeof(duk__test_double_union) != 8) {
		DUK__FAILED("invalid union size");
	}

	return error_count;
}

/*
 *  Union aliasing, see misc/clang_aliasing.c.
 */

DUK_LOCAL duk_uint_t duk__selftest_double_aliasing(void) {
	/* This testcase fails when Emscripten-generated code runs on Firefox.
	 * It's not an issue because the failure should only affect packed
	 * duk_tval representation, which is not used with Emscripten.
	 */
#if defined(DUK_USE_PACKED_TVAL)
	duk_uint_t error_count = 0;
	duk__test_double_union a, b;

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

	return error_count;
#else
	DUK_D(DUK_DPRINT("skip double aliasing self test when duk_tval is not packed"));
	return 0;
#endif
}

/*
 *  Zero sign, see misc/tcc_zerosign2.c.
 */

DUK_LOCAL duk_uint_t duk__selftest_double_zero_sign(void) {
	duk_uint_t error_count = 0;
	duk__test_double_union a, b;

	a.d = 0.0;
	b.d = -a.d;
	DUK__DBLUNION_CMP_FALSE(&a, &b);

	return error_count;
}

/*
 *  Struct size/alignment if platform requires it
 *
 *  There are some compiler specific struct padding pragmas etc in use, this
 *  selftest ensures they're correctly detected and used.
 */

DUK_LOCAL duk_uint_t duk__selftest_struct_align(void) {
	duk_uint_t error_count = 0;

#if (DUK_USE_ALIGN_BY == 4)
	if ((sizeof(duk_hbuffer_fixed) % 4) != 0) {
		DUK__FAILED("sizeof(duk_hbuffer_fixed) not aligned to 4");
	}
#elif (DUK_USE_ALIGN_BY == 8)
	if ((sizeof(duk_hbuffer_fixed) % 8) != 0) {
		DUK__FAILED("sizeof(duk_hbuffer_fixed) not aligned to 8");
	}
#elif (DUK_USE_ALIGN_BY == 1)
	/* no check */
#else
#error invalid DUK_USE_ALIGN_BY
#endif
	return error_count;
}

/*
 *  64-bit arithmetic
 *
 *  There are some platforms/compilers where 64-bit types are available
 *  but don't work correctly.  Test for known cases.
 */

DUK_LOCAL duk_uint_t duk__selftest_64bit_arithmetic(void) {
	duk_uint_t error_count = 0;
#if defined(DUK_USE_64BIT_OPS)
	volatile duk_int64_t i;
	volatile duk_double_t d;

	/* Catch a double-to-int64 cast issue encountered in practice. */
	d = 2147483648.0;
	i = (duk_int64_t) d;
	if (i != 0x80000000LL) {
		DUK__FAILED("casting 2147483648.0 to duk_int64_t failed");
	}
#else
	/* nop */
#endif
	return error_count;
}

/*
 *  Casting
 */

DUK_LOCAL duk_uint_t duk__selftest_cast_double_to_small_uint(void) {
	/*
	 *  https://github.com/svaarala/duktape/issues/127#issuecomment-77863473
	 */

	duk_uint_t error_count = 0;

	duk_double_t d1, d2;
	duk_small_uint_t u;

	duk_double_t d1v, d2v;
	duk_small_uint_t uv;

	/* Test without volatiles */

	d1 = 1.0;
	u = (duk_small_uint_t) d1;
	d2 = (duk_double_t) u;

	if (!(d1 == 1.0 && u == 1 && d2 == 1.0 && d1 == d2)) {
		DUK__FAILED("double to duk_small_uint_t cast failed");
	}

	/* Same test with volatiles */

	d1v = 1.0;
	uv = (duk_small_uint_t) d1v;
	d2v = (duk_double_t) uv;

	if (!(d1v == 1.0 && uv == 1 && d2v == 1.0 && d1v == d2v)) {
		DUK__FAILED("double to duk_small_uint_t cast failed");
	}

	return error_count;
}

DUK_LOCAL duk_uint_t duk__selftest_cast_double_to_uint32(void) {
	/*
	 *  This test fails on an exotic ARM target; double-to-uint
	 *  cast is incorrectly clamped to -signed- int highest value.
	 *
	 *  https://github.com/svaarala/duktape/issues/336
	 */

	duk_uint_t error_count = 0;
	duk_double_t dv;
	duk_uint32_t uv;

	dv = 3735928559.0;  /* 0xdeadbeef in decimal */
	uv = (duk_uint32_t) dv;

	if (uv != 0xdeadbeefUL) {
		DUK__FAILED("double to duk_uint32_t cast failed");
	}

	return error_count;
}

/*
 *  Self test main
 */

DUK_INTERNAL duk_uint_t duk_selftest_run_tests(void) {
	duk_uint_t error_count = 0;

	DUK_D(DUK_DPRINT("self test starting"));

	error_count += duk__selftest_types();
	error_count += duk__selftest_packed_tval();
	error_count += duk__selftest_twos_complement();
	error_count += duk__selftest_byte_order();
	error_count += duk__selftest_bswap_macros();
	error_count += duk__selftest_double_union_size();
	error_count += duk__selftest_double_aliasing();
	error_count += duk__selftest_double_zero_sign();
	error_count += duk__selftest_struct_align();
	error_count += duk__selftest_64bit_arithmetic();
	error_count += duk__selftest_cast_double_to_small_uint();
	error_count += duk__selftest_cast_double_to_uint32();

	DUK_D(DUK_DPRINT("self test complete, total error count: %ld", (long) error_count));

	return error_count;
}

#endif  /* DUK_USE_SELF_TESTS */
