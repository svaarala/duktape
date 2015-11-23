/*===
*** test_1 (duk_safe_call)
top: 29
index 0, number: nan
index 1, number: 0.000000
index 2, number: 1.000000
index 3, number: 0.000000
index 4, number: 1.000000
index 5, number: -123.456000
index 6, number: nan
index 7, number: inf
index 8, number: 0.000000
index 9, number: nan
index 10, number: 123.000000
index 11, number: 123.456000
index 12, number: 123456.000000
index 13, number: -123456.000000
index 14, number: nan
index 15, number: -inf
index 16, number: inf
index 17, number: inf
index 18, number: nan
index 19, number: nan
index 20, number: inf
index 21, number: nan
index 22, number: nan
index 23, number: 0.000000
index 24, number: nan
index 25, number: 0.000000
index 26, number: nan
index 27, number: 0.000000
index 28, number: 1.000000
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
==> rc=1, result='Error: invalid stack index 3'
*** test_3 (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	duk_idx_t i, n;

	duk_set_top(ctx, 0);

	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_true(ctx);
	duk_push_false(ctx);
	duk_push_int(ctx, 1);
	duk_push_number(ctx, -123.456);
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
	duk_push_fixed_buffer(ctx, 0);    /* coerces like string: ToNumber('') = 0 */
	duk_push_fixed_buffer(ctx, 1024); /* coerces like string: ToNumber('\u0000\u0000...') = NaN */
	duk_push_dynamic_buffer(ctx, 0);
	duk_push_dynamic_buffer(ctx, 1024);
	duk_push_pointer(ctx, (void *) NULL);
	duk_push_pointer(ctx, (void *) 0xdeadbeef);

	n = duk_get_top(ctx);
	printf("top: %ld\n", (long) n);
	for (i = 0; i < n; i++) {
		printf("index %ld, number: %lf\n", (long) i, (double) duk_to_number(ctx, i));
	}

	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_number(ctx, 3);
	printf("index 3 OK\n");
	return 0;
}

static duk_ret_t test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_number(ctx, DUK_INVALID_INDEX);
	printf("index DUK_INVALID_INDEX OK\n");
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
}
