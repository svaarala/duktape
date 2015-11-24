/*===
*** test_1 (duk_safe_call)
top: 33
index 0, int: 0, number before: nan, number after: 0.000000
index 0, uint: 0, number before: nan, number after: 0.000000
index 1, int: 0, number before: nan, number after: 0.000000
index 1, uint: 0, number before: nan, number after: 0.000000
index 2, int: 1, number before: nan, number after: 1.000000
index 2, uint: 1, number before: nan, number after: 1.000000
index 3, int: 0, number before: nan, number after: 0.000000
index 3, uint: 0, number before: nan, number after: 0.000000
index 4, int: 0, number before: 0.000000, number after: 0.000000
index 4, uint: 0, number before: 0.000000, number after: 0.000000
index 5, int: 1, number before: 1.000000, number after: 1.000000
index 5, uint: 1, number before: 1.000000, number after: 1.000000
index 6, int: 123, number before: 123.456000, number after: 123.000000
index 6, uint: 123, number before: 123.456000, number after: 123.000000
index 7, int: -123, number before: -123.456000, number after: -123.000000
index 7, uint: 0, number before: -123.456000, number after: -123.000000
index 8, int: 123, number before: 123.999000, number after: 123.000000
index 8, uint: 123, number before: 123.999000, number after: 123.000000
index 9, int: -123, number before: -123.999000, number after: -123.000000
index 9, uint: 0, number before: -123.999000, number after: -123.000000
index 10, int: 0, number before: nan, number after: 0.000000
index 10, uint: 0, number before: nan, number after: 0.000000
index 11, int: DUK_INT_MAX, number before: inf, number after: inf
index 11, uint: DUK_UINT_MAX, number before: inf, number after: inf
index 12, int: 0, number before: nan, number after: 0.000000
index 12, uint: 0, number before: nan, number after: 0.000000
index 13, int: 0, number before: nan, number after: 0.000000
index 13, uint: 0, number before: nan, number after: 0.000000
index 14, int: 123, number before: nan, number after: 123.000000
index 14, uint: 123, number before: nan, number after: 123.000000
index 15, int: 123, number before: nan, number after: 123.000000
index 15, uint: 123, number before: nan, number after: 123.000000
index 16, int: 123456, number before: nan, number after: 123456.000000
index 16, uint: 123456, number before: nan, number after: 123456.000000
index 17, int: -123456, number before: nan, number after: -123456.000000
index 17, uint: 0, number before: nan, number after: -123456.000000
index 18, int: 0, number before: nan, number after: 0.000000
index 18, uint: 0, number before: nan, number after: 0.000000
index 19, int: DUK_INT_MIN, number before: nan, number after: -inf
index 19, uint: 0, number before: nan, number after: -inf
index 20, int: DUK_INT_MAX, number before: nan, number after: inf
index 20, uint: DUK_UINT_MAX, number before: nan, number after: inf
index 21, int: DUK_INT_MAX, number before: nan, number after: inf
index 21, uint: DUK_UINT_MAX, number before: nan, number after: inf
index 22, int: 0, number before: nan, number after: 0.000000
index 22, uint: 0, number before: nan, number after: 0.000000
index 23, int: 0, number before: nan, number after: 0.000000
index 23, uint: 0, number before: nan, number after: 0.000000
index 24, int: DUK_INT_MAX, number before: nan, number after: inf
index 24, uint: DUK_UINT_MAX, number before: nan, number after: inf
index 25, int: 0, number before: nan, number after: 0.000000
index 25, uint: 0, number before: nan, number after: 0.000000
index 26, int: 0, number before: nan, number after: 0.000000
index 26, uint: 0, number before: nan, number after: 0.000000
index 27, int: 0, number before: nan, number after: 0.000000
index 27, uint: 0, number before: nan, number after: 0.000000
index 28, int: 0, number before: nan, number after: 0.000000
index 28, uint: 0, number before: nan, number after: 0.000000
index 29, int: 0, number before: nan, number after: 0.000000
index 29, uint: 0, number before: nan, number after: 0.000000
index 30, int: 0, number before: nan, number after: 0.000000
index 30, uint: 0, number before: nan, number after: 0.000000
index 31, int: 0, number before: nan, number after: 0.000000
index 31, uint: 0, number before: nan, number after: 0.000000
index 32, int: 1, number before: nan, number after: 1.000000
index 32, uint: 1, number before: nan, number after: 1.000000
==> rc=0, result='undefined'
*** test_2a (duk_safe_call)
==> rc=1, result='Error: invalid stack index 3'
*** test_2b (duk_safe_call)
==> rc=1, result='Error: invalid stack index 3'
*** test_3a (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
*** test_3b (duk_safe_call)
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
	duk_push_number(ctx, 123.456);
	duk_push_number(ctx, -123.456);
	duk_push_number(ctx, 123.999);
	duk_push_number(ctx, -123.999);

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
	duk_push_fixed_buffer(ctx, 0);
	duk_push_fixed_buffer(ctx, 1024);
	duk_push_dynamic_buffer(ctx, 0);

	duk_push_dynamic_buffer(ctx, 1024);
	duk_push_pointer(ctx, (void *) NULL);
	duk_push_pointer(ctx, (void *) 0xdeadbeef);

#if 0
	printf("%lld %lld %llu %llu\n",
		(long long int) DUK_INT_MIN,
		(long long int) DUK_INT_MAX,
		(unsigned long long int) DUK_UINT_MIN,
		(unsigned long long int) DUK_UINT_MAX);
#endif

	n = duk_get_top(ctx);
	printf("top: %ld\n", (long) n);
	for (i = 0; i < n; i++) {
		duk_double_t dval_pre;
		duk_double_t dval_post;
		duk_int_t ival;
		duk_uint_t uval;

		/* duk_to_int() */
		duk_dup(ctx, i);
		dval_pre = duk_get_number(ctx, -1);    /* number before ToInteger() coercion */
		ival = duk_to_int(ctx, -1);
		dval_post = duk_get_number(ctx, -1);   /* number after ToInteger() coercion */
		printf("index %ld, ", (long) i);
		if (ival == DUK_INT_MIN) {
			printf("int: DUK_INT_MIN");
		} else if (ival == DUK_INT_MAX) {
			printf("int: DUK_INT_MAX");
		} else {
			printf("int: %ld", (long) ival);
		}
		printf(", number before: %lf, number after: %lf\n", (double) dval_pre, (double) dval_post);
		duk_pop(ctx);

		/* duk_to_uint() */
		duk_dup(ctx, i);
		dval_pre = duk_get_number(ctx, -1);    /* number before ToInteger() coercion */
		uval = duk_to_uint(ctx, -1);
		dval_post = duk_get_number(ctx, -1);   /* number after ToInteger() coercion */
		printf("index %ld, ", (long) i);
		if (uval == DUK_UINT_MAX) {
			printf("uint: DUK_UINT_MAX");
		} else {
			printf("uint: %lu", (unsigned long) uval);
		}
		printf(", number before: %lf, number after: %lf\n", (double) dval_pre, (double) dval_post);
		duk_pop(ctx);
	}

	return 0;
}

static duk_ret_t test_2a(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_int(ctx, 3);
	printf("index 3 OK\n");
	return 0;
}

static duk_ret_t test_2b(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_uint(ctx, 3);
	printf("index 3 OK\n");
	return 0;
}

static duk_ret_t test_3a(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_int(ctx, DUK_INVALID_INDEX);
	printf("index DUK_INVALID_INDEX OK\n");
	return 0;
}

static duk_ret_t test_3b(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_uint(ctx, DUK_INVALID_INDEX);
	printf("index DUK_INVALID_INDEX OK\n");
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2a);
	TEST_SAFE_CALL(test_2b);
	TEST_SAFE_CALL(test_3a);
	TEST_SAFE_CALL(test_3b);
}
