/*===
top: 33
index 0, int: 0, number before: nan, number after: 0.000000
index 1, int: 0, number before: nan, number after: 0.000000
index 2, int: 1, number before: nan, number after: 1.000000
index 3, int: 0, number before: nan, number after: 0.000000
index 4, int: 0, number before: 0.000000, number after: 0.000000
index 5, int: 1, number before: 1.000000, number after: 1.000000
index 6, int: 123, number before: 123.456000, number after: 123.000000
index 7, int: -123, number before: -123.456000, number after: -123.000000
index 8, int: 123, number before: 123.999000, number after: 123.000000
index 9, int: -123, number before: -123.999000, number after: -123.000000
index 10, int: 0, number before: nan, number after: 0.000000
index 11, int: INT_MAX, number before: inf, number after: inf
index 12, int: 0, number before: nan, number after: 0.000000
index 13, int: 0, number before: nan, number after: 0.000000
index 14, int: 123, number before: nan, number after: 123.000000
index 15, int: 123, number before: nan, number after: 123.000000
index 16, int: 123456, number before: nan, number after: 123456.000000
index 17, int: -123456, number before: nan, number after: -123456.000000
index 18, int: 0, number before: nan, number after: 0.000000
index 19, int: INT_MIN, number before: nan, number after: -inf
index 20, int: INT_MAX, number before: nan, number after: inf
index 21, int: INT_MAX, number before: nan, number after: inf
index 22, int: 0, number before: nan, number after: 0.000000
index 23, int: 0, number before: nan, number after: 0.000000
index 24, int: INT_MAX, number before: nan, number after: inf
index 25, int: 0, number before: nan, number after: 0.000000
index 26, int: 0, number before: nan, number after: 0.000000
index 27, int: 0, number before: nan, number after: 0.000000
index 28, int: 0, number before: nan, number after: 0.000000
index 29, int: 0, number before: nan, number after: 0.000000
index 30, int: 0, number before: nan, number after: 0.000000
index 31, int: 0, number before: nan, number after: 0.000000
index 32, int: 1, number before: nan, number after: 1.000000
rc=0, result=undefined
rc=1, result=Error: index out of bounds
rc=1, result=Error: index out of bounds
===*/

int test_1(duk_context *ctx) {
	int i, n;

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

	n = duk_get_top(ctx);
	printf("top: %d\n", n);
	for (i = 0; i < n; i++) {
		double dval_pre;
		double dval_post;
		int ival;

		dval_pre = duk_get_number(ctx, i);  /* number before ToInteger() coercion */
		ival = duk_to_int(ctx, i);
		dval_post = duk_get_number(ctx, i);      /* number after ToInteger() coercion */
		printf("index %d, ", i);
		if (ival == INT_MIN) {
			printf("int: INT_MIN");
		} else if (ival == INT_MAX) {
			printf("int: INT_MAX");
		} else {
			printf("int: %d", ival);
		}
		printf(", number before: %lf, number after: %lf\n", dval_pre, dval_post);
	}

	return 0;
}

int test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_int(ctx, 3);
	printf("index 3 OK\n");
	return 0;
}

int test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_int(ctx, DUK_INVALID_INDEX);
	printf("index DUK_INVALID_INDEX OK\n");
	return 0;
}

void test(duk_context *ctx) {
	int rc;

	rc = duk_safe_call(ctx, test_1, 0, 1);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_2, 0, 1);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_3, 0, 1);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);
}

