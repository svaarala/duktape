/*===
*** test_1 (duk_safe_call)
top: 43
index 0, int32: 0, number before: nan, number after: 0.000000
index 0, uint32: 0, number before: nan, number after: 0.000000
index 0, uint16: 0, number before: nan, number after: 0.000000
index 1, int32: 0, number before: nan, number after: 0.000000
index 1, uint32: 0, number before: nan, number after: 0.000000
index 1, uint16: 0, number before: nan, number after: 0.000000
index 2, int32: 1, number before: nan, number after: 1.000000
index 2, uint32: 1, number before: nan, number after: 1.000000
index 2, uint16: 1, number before: nan, number after: 1.000000
index 3, int32: 0, number before: nan, number after: 0.000000
index 3, uint32: 0, number before: nan, number after: 0.000000
index 3, uint16: 0, number before: nan, number after: 0.000000
index 4, int32: 0, number before: 0.000000, number after: 0.000000
index 4, uint32: 0, number before: 0.000000, number after: 0.000000
index 4, uint16: 0, number before: 0.000000, number after: 0.000000
index 5, int32: 1, number before: 1.000000, number after: 1.000000
index 5, uint32: 1, number before: 1.000000, number after: 1.000000
index 5, uint16: 1, number before: 1.000000, number after: 1.000000
index 6, int32: -1, number before: -1.000000, number after: -1.000000
index 6, uint32: 4294967295, number before: -1.000000, number after: 4294967295.000000
index 6, uint16: 65535, number before: -1.000000, number after: 65535.000000
index 7, int32: 123, number before: 123.456000, number after: 123.000000
index 7, uint32: 123, number before: 123.456000, number after: 123.000000
index 7, uint16: 123, number before: 123.456000, number after: 123.000000
index 8, int32: -123, number before: -123.456000, number after: -123.000000
index 8, uint32: 4294967173, number before: -123.456000, number after: 4294967173.000000
index 8, uint16: 65413, number before: -123.456000, number after: 65413.000000
index 9, int32: 123, number before: 123.999000, number after: 123.000000
index 9, uint32: 123, number before: 123.999000, number after: 123.000000
index 9, uint16: 123, number before: 123.999000, number after: 123.000000
index 10, int32: -123, number before: -123.999000, number after: -123.000000
index 10, uint32: 4294967173, number before: -123.999000, number after: 4294967173.000000
index 10, uint16: 65413, number before: -123.999000, number after: 65413.000000
index 11, int32: 2147483647, number before: -2147483649.000000, number after: 2147483647.000000
index 11, uint32: 2147483647, number before: -2147483649.000000, number after: 2147483647.000000
index 11, uint16: 65535, number before: -2147483649.000000, number after: 65535.000000
index 12, int32: -2147483648, number before: -2147483648.000000, number after: -2147483648.000000
index 12, uint32: 2147483648, number before: -2147483648.000000, number after: 2147483648.000000
index 12, uint16: 0, number before: -2147483648.000000, number after: 0.000000
index 13, int32: 2147483647, number before: 2147483647.000000, number after: 2147483647.000000
index 13, uint32: 2147483647, number before: 2147483647.000000, number after: 2147483647.000000
index 13, uint16: 65535, number before: 2147483647.000000, number after: 65535.000000
index 14, int32: -2147483648, number before: 2147483648.000000, number after: -2147483648.000000
index 14, uint32: 2147483648, number before: 2147483648.000000, number after: 2147483648.000000
index 14, uint16: 0, number before: 2147483648.000000, number after: 0.000000
index 15, int32: -1, number before: 4294967295.000000, number after: -1.000000
index 15, uint32: 4294967295, number before: 4294967295.000000, number after: 4294967295.000000
index 15, uint16: 65535, number before: 4294967295.000000, number after: 65535.000000
index 16, int32: 0, number before: 4294967296.000000, number after: 0.000000
index 16, uint32: 0, number before: 4294967296.000000, number after: 0.000000
index 16, uint16: 0, number before: 4294967296.000000, number after: 0.000000
index 17, int32: 65535, number before: 65535.000000, number after: 65535.000000
index 17, uint32: 65535, number before: 65535.000000, number after: 65535.000000
index 17, uint16: 65535, number before: 65535.000000, number after: 65535.000000
index 18, int32: 65536, number before: 65536.000000, number after: 65536.000000
index 18, uint32: 65536, number before: 65536.000000, number after: 65536.000000
index 18, uint16: 0, number before: 65536.000000, number after: 0.000000
index 19, int32: 1410065407, number before: 9999999999.000000, number after: 1410065407.000000
index 19, uint32: 1410065407, number before: 9999999999.000000, number after: 1410065407.000000
index 19, uint16: 58367, number before: 9999999999.000000, number after: 58367.000000
index 20, int32: 0, number before: nan, number after: 0.000000
index 20, uint32: 0, number before: nan, number after: 0.000000
index 20, uint16: 0, number before: nan, number after: 0.000000
index 21, int32: 0, number before: inf, number after: 0.000000
index 21, uint32: 0, number before: inf, number after: 0.000000
index 21, uint16: 0, number before: inf, number after: 0.000000
index 22, int32: 0, number before: nan, number after: 0.000000
index 22, uint32: 0, number before: nan, number after: 0.000000
index 22, uint16: 0, number before: nan, number after: 0.000000
index 23, int32: 0, number before: nan, number after: 0.000000
index 23, uint32: 0, number before: nan, number after: 0.000000
index 23, uint16: 0, number before: nan, number after: 0.000000
index 24, int32: 123, number before: nan, number after: 123.000000
index 24, uint32: 123, number before: nan, number after: 123.000000
index 24, uint16: 123, number before: nan, number after: 123.000000
index 25, int32: 123, number before: nan, number after: 123.000000
index 25, uint32: 123, number before: nan, number after: 123.000000
index 25, uint16: 123, number before: nan, number after: 123.000000
index 26, int32: 123456, number before: nan, number after: 123456.000000
index 26, uint32: 123456, number before: nan, number after: 123456.000000
index 26, uint16: 57920, number before: nan, number after: 57920.000000
index 27, int32: -123456, number before: nan, number after: -123456.000000
index 27, uint32: 4294843840, number before: nan, number after: 4294843840.000000
index 27, uint16: 7616, number before: nan, number after: 7616.000000
index 28, int32: 0, number before: nan, number after: 0.000000
index 28, uint32: 0, number before: nan, number after: 0.000000
index 28, uint16: 0, number before: nan, number after: 0.000000
index 29, int32: 0, number before: nan, number after: 0.000000
index 29, uint32: 0, number before: nan, number after: 0.000000
index 29, uint16: 0, number before: nan, number after: 0.000000
index 30, int32: 0, number before: nan, number after: 0.000000
index 30, uint32: 0, number before: nan, number after: 0.000000
index 30, uint16: 0, number before: nan, number after: 0.000000
index 31, int32: 0, number before: nan, number after: 0.000000
index 31, uint32: 0, number before: nan, number after: 0.000000
index 31, uint16: 0, number before: nan, number after: 0.000000
index 32, int32: 0, number before: nan, number after: 0.000000
index 32, uint32: 0, number before: nan, number after: 0.000000
index 32, uint16: 0, number before: nan, number after: 0.000000
index 33, int32: 0, number before: nan, number after: 0.000000
index 33, uint32: 0, number before: nan, number after: 0.000000
index 33, uint16: 0, number before: nan, number after: 0.000000
index 34, int32: 0, number before: nan, number after: 0.000000
index 34, uint32: 0, number before: nan, number after: 0.000000
index 34, uint16: 0, number before: nan, number after: 0.000000
index 35, int32: 0, number before: nan, number after: 0.000000
index 35, uint32: 0, number before: nan, number after: 0.000000
index 35, uint16: 0, number before: nan, number after: 0.000000
index 36, int32: 0, number before: nan, number after: 0.000000
index 36, uint32: 0, number before: nan, number after: 0.000000
index 36, uint16: 0, number before: nan, number after: 0.000000
index 37, int32: 0, number before: nan, number after: 0.000000
index 37, uint32: 0, number before: nan, number after: 0.000000
index 37, uint16: 0, number before: nan, number after: 0.000000
index 38, int32: 0, number before: nan, number after: 0.000000
index 38, uint32: 0, number before: nan, number after: 0.000000
index 38, uint16: 0, number before: nan, number after: 0.000000
index 39, int32: 0, number before: nan, number after: 0.000000
index 39, uint32: 0, number before: nan, number after: 0.000000
index 39, uint16: 0, number before: nan, number after: 0.000000
index 40, int32: 0, number before: nan, number after: 0.000000
index 40, uint32: 0, number before: nan, number after: 0.000000
index 40, uint16: 0, number before: nan, number after: 0.000000
index 41, int32: 0, number before: nan, number after: 0.000000
index 41, uint32: 0, number before: nan, number after: 0.000000
index 41, uint16: 0, number before: nan, number after: 0.000000
index 42, int32: 1, number before: nan, number after: 1.000000
index 42, uint32: 1, number before: nan, number after: 1.000000
index 42, uint16: 1, number before: nan, number after: 1.000000
==> rc=0, result='undefined'
*** test_2a (duk_safe_call)
==> rc=1, result='Error: invalid stack index 3'
*** test_2b (duk_safe_call)
==> rc=1, result='Error: invalid stack index 3'
*** test_2c (duk_safe_call)
==> rc=1, result='Error: invalid stack index 3'
*** test_3a (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
*** test_3b (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
*** test_3c (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	duk_idx_t i, n;

	duk_set_top(ctx, 0);

	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_true(ctx);
	duk_push_false(ctx);
	duk_push_int(ctx, 0);
	duk_push_int(ctx, 1);
	duk_push_int(ctx, -1);
	duk_push_number(ctx, 123.456);
	duk_push_number(ctx, -123.456);
	duk_push_number(ctx, 123.999);

	duk_push_number(ctx, -123.999);
	duk_push_number(ctx, -2147483649.0); /* min int32 - 1 */
	duk_push_number(ctx, -2147483648.0); /* min int32 */
	duk_push_number(ctx, 2147483647.0);  /* max int32 */
	duk_push_number(ctx, 2147483648.0);  /* max int32 + 1 */
	duk_push_number(ctx, 4294967295.0);  /* max uint32 */
	duk_push_number(ctx, 4294967296.0);  /* max uint32 + 1 */
	duk_push_number(ctx, 65535.0);       /* max uint16 */
	duk_push_number(ctx, 65536.0);       /* max uint16 + 1 */
	duk_push_number(ctx, 9999999999.0);

	duk_push_nan(ctx);
	duk_push_number(ctx, INFINITY);
	duk_push_string(ctx, "");
	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "123");
	duk_push_string(ctx, "123.456");
	duk_push_string(ctx, "123.456e3");
	duk_push_string(ctx, "  -123.456e+3  ");
	duk_push_string(ctx, "NaN");
	duk_push_string(ctx, "-Infinity");

	duk_push_string(ctx, "+Infinity");
	duk_push_string(ctx, "Infinity");
	duk_push_string(ctx, "Infinityx");
	duk_push_string(ctx, "xInfinity");
	duk_push_string(ctx, "  Infinity  ");
	duk_push_object(ctx);
	duk_push_thread(ctx);
	duk_push_fixed_buffer(ctx, 0);     /* ToNumber('') = 0 */
	duk_push_fixed_buffer(ctx, 1024);  /* ToNumber('\u0000...') = NaN, converts into 0 when doing integer coercion */
	duk_push_dynamic_buffer(ctx, 0);

	duk_push_dynamic_buffer(ctx, 1024);
	duk_push_pointer(ctx, (void *) NULL);
	duk_push_pointer(ctx, (void *) 0xdeadbeef);

	n = duk_get_top(ctx);
	printf("top: %ld\n", (long) n);
	for (i = 0; i < n; i++) {
		duk_double_t dval_pre;
		duk_double_t dval_post;
		duk_int_t ival;
		duk_uint_t uval;

		duk_dup(ctx, i);
		dval_pre = duk_get_number(ctx, -1);
		ival = duk_to_int32(ctx, -1);
		dval_post = duk_get_number(ctx, -1);
		printf("index %ld, int32: %ld, number before: %lf, number after: %lf\n",
		       (long) i, (long) ival, (double) dval_pre, (double) dval_post);
		duk_pop(ctx);

		duk_dup(ctx, i);
		dval_pre = duk_get_number(ctx, -1);
		uval = duk_to_uint32(ctx, -1);
		dval_post = duk_get_number(ctx, -1);
		printf("index %ld, uint32: %lu, number before: %lf, number after: %lf\n",
		       (long) i, (unsigned long) uval, (double) dval_pre, (double) dval_post);
		duk_pop(ctx);

		duk_dup(ctx, i);
		dval_pre = duk_get_number(ctx, -1);
		uval = duk_to_uint16(ctx, -1);
		dval_post = duk_get_number(ctx, -1);
		printf("index %ld, uint16: %lu, number before: %lf, number after: %lf\n",
		       (long) i, (unsigned long) uval, (double) dval_pre, (double) dval_post);
		duk_pop(ctx);
	}

	return 0;
}

static duk_ret_t test_2a(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_int32(ctx, 3);
	printf("index 3 OK\n");
	return 0;
}

static duk_ret_t test_2b(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_uint32(ctx, 3);
	printf("index 3 OK\n");
	return 0;
}

static duk_ret_t test_2c(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_uint16(ctx, 3);
	printf("index 3 OK\n");
	return 0;
}

static duk_ret_t test_3a(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_int32(ctx, DUK_INVALID_INDEX);
	printf("index DUK_INVALID_INDEX OK\n");
	return 0;
}

static duk_ret_t test_3b(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_uint32(ctx, DUK_INVALID_INDEX);
	printf("index DUK_INVALID_INDEX OK\n");
	return 0;
}

static duk_ret_t test_3c(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_uint16(ctx, DUK_INVALID_INDEX);
	printf("index DUK_INVALID_INDEX OK\n");
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2a);
	TEST_SAFE_CALL(test_2b);
	TEST_SAFE_CALL(test_2c);
	TEST_SAFE_CALL(test_3a);
	TEST_SAFE_CALL(test_3b);
	TEST_SAFE_CALL(test_3c);
}
