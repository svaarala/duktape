/*===
*** test_1 (duk_safe_call)
top: 10
index 0, type: 6, string coerced: true
index 1, type: 6, string coerced: false
index 2, type: 6, string coerced: 0
index 3, type: 6, string coerced: 1
index 4, type: 6, string coerced: NaN
index 5, type: 6, string coerced: Infinity
index 6, type: 6, string coerced: 
index 7, type: 6, string coerced: foo
index 8, type: 6, string coerced: [object Object]
index 9, type: 6, string coerced: [object Thread]
==> rc=0, result='undefined'
*** test_2a (duk_safe_call)
==> rc=1, result='TypeError: attempt to coerce incompatible value to object'
*** test_2b (duk_safe_call)
==> rc=1, result='TypeError: attempt to coerce incompatible value to object'
*** test_2c (duk_safe_call)
==> rc=1, result='TypeError: attempt to coerce incompatible value to object'
*** test_2d (duk_safe_call)
==> rc=1, result='TypeError: attempt to coerce incompatible value to object'
*** test_2e (duk_safe_call)
==> rc=1, result='TypeError: attempt to coerce incompatible value to object'
*** test_2f (duk_safe_call)
==> rc=1, result='TypeError: attempt to coerce incompatible value to object'
*** test_2g (duk_safe_call)
==> rc=1, result='TypeError: attempt to coerce incompatible value to object'
*** test_2h (duk_safe_call)
==> rc=1, result='TypeError: attempt to coerce incompatible value to object'
*** test_3 (duk_safe_call)
==> rc=1, result='Error: invalid index: 3'
*** test_4 (duk_safe_call)
==> rc=1, result='Error: invalid index: -2147483648'
===*/

int test_1(duk_context *ctx) {
	int i, n;

	duk_set_top(ctx, 0);
	duk_push_true(ctx);
	duk_push_false(ctx);
	duk_push_int(ctx, 0);
	duk_push_int(ctx, 1);
	duk_push_nan(ctx);
	duk_push_number(ctx, INFINITY);
	duk_push_string(ctx, "");
	duk_push_string(ctx, "foo");
	duk_push_object(ctx);
	duk_push_thread(ctx);

	n = duk_get_top(ctx);
	printf("top: %d\n", n);
	for (i = 0; i < n; i++) {
		int t;
		duk_to_object(ctx, i);
		t = duk_get_type(ctx, i);
		duk_to_string(ctx, i);
		printf("index %d, type: %d, string coerced: %s\n", i, t, duk_to_string(ctx, i));
	}

	return 0;
}

/* non-coercible types */
int test_2a(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_undefined(ctx);
	duk_to_object(ctx, 0);
	printf("index 0 OK\n");
	return 0;
}
int test_2b(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_null(ctx);
	duk_to_object(ctx, 0);
	printf("index 0 OK\n");
	return 0;
}
int test_2c(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_fixed_buffer(ctx, 0);
	duk_to_object(ctx, 0);
	printf("index 0 OK\n");
	return 0;
}
int test_2d(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_fixed_buffer(ctx, 1024);
	duk_to_object(ctx, 0);
	printf("index 0 OK\n");
	return 0;
}
int test_2e(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_dynamic_buffer(ctx, 0);
	duk_to_object(ctx, 0);
	printf("index 0 OK\n");
	return 0;
}
int test_2f(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_dynamic_buffer(ctx, 1024);
	duk_to_object(ctx, 0);
	printf("index 0 OK\n");
	return 0;
}
int test_2g(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_pointer(ctx, (void *) NULL);
	duk_to_object(ctx, 0);
	printf("index 0 OK\n");
	return 0;
}
int test_2h(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_pointer(ctx, (void *) 0xdeadbeef);
	duk_to_object(ctx, 0);
	printf("index 0 OK\n");
	return 0;
}

int test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_boolean(ctx, 3);
	printf("index 3 OK\n");
	return 0;
}

int test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_boolean(ctx, DUK_INVALID_INDEX);
	printf("index DUK_INVALID_INDEX OK\n");
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2a);
	TEST_SAFE_CALL(test_2b);
	TEST_SAFE_CALL(test_2c);
	TEST_SAFE_CALL(test_2d);
	TEST_SAFE_CALL(test_2e);
	TEST_SAFE_CALL(test_2f);
	TEST_SAFE_CALL(test_2g);
	TEST_SAFE_CALL(test_2h);
	TEST_SAFE_CALL(test_3);

	/* FIXME: this testcase currently exposes the DUK_INVALID_INDEX
	 * constant in the error message and is thus not portable.
	 */
	TEST_SAFE_CALL(test_4);
}

