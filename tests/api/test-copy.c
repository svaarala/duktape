/*===
*** test_1 (duk_safe_call)
0: 123
1: 234
2: 123
3: foo
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
0: 123
1: 234
2: 345
3: foo
==> rc=0, result='undefined'
*** test_3a (duk_safe_call)
==> rc=1, result='Error: invalid stack index -5'
*** test_3b (duk_safe_call)
==> rc=1, result='Error: invalid stack index 5'
*** test_3c (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
*** test_3d (duk_safe_call)
==> rc=1, result='Error: invalid stack index -5'
*** test_3e (duk_safe_call)
==> rc=1, result='Error: invalid stack index 5'
*** test_3f (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
*** test_3g (duk_safe_call)
==> rc=1, result='Error: invalid stack index 10'
===*/

static void prep(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_push_int(ctx, 345);       /* -> [ 123 234 345 ] */
	duk_push_string(ctx, "foo");  /* -> [ 123 234 345 "foo" ] */
}

static duk_ret_t test_1(duk_context *ctx) {
	duk_idx_t i, n;

	prep(ctx);
	duk_copy(ctx, -4, 2);         /* -> [ 123 234 123 "foo" ] */

	n = duk_get_top(ctx);
	for (i = 0; i < n; i++) {
		printf("%ld: %s\n", (long) i, duk_to_string(ctx, i));
	}

	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	duk_idx_t i, n;

	prep(ctx);
	duk_copy(ctx, -3, -3);  /* nop */

	n = duk_get_top(ctx);
	for (i = 0; i < n; i++) {
		printf("%ld: %s\n", (long) i, duk_to_string(ctx, i));
	}

	return 0;
}

static duk_ret_t test_3a(duk_context *ctx) {
	prep(ctx);
	duk_copy(ctx, -5, 2);  /* source index too low */
	return 0;
}

static duk_ret_t test_3b(duk_context *ctx) {
	prep(ctx);
	duk_copy(ctx, 5, 2);  /* source index too high */
	return 0;
}

static duk_ret_t test_3c(duk_context *ctx) {
	prep(ctx);
	duk_copy(ctx, DUK_INVALID_INDEX, 2);  /* source index invalid */
	return 0;
}

static duk_ret_t test_3d(duk_context *ctx) {
	prep(ctx);
	duk_copy(ctx, 2, -5);  /* source index too low */
	return 0;
}

static duk_ret_t test_3e(duk_context *ctx) {
	prep(ctx);
	duk_copy(ctx, 2, 5);  /* source index too high */
	return 0;
}

static duk_ret_t test_3f(duk_context *ctx) {
	prep(ctx);
	duk_copy(ctx, 2, DUK_INVALID_INDEX);  /* source index invalid */
	return 0;
}

static duk_ret_t test_3g(duk_context *ctx) {
	prep(ctx);
	/* indices both invalid and equal: would be nop, but still not allowed for invalid indices */
	duk_copy(ctx, 10, 10);
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3a);
	TEST_SAFE_CALL(test_3b);
	TEST_SAFE_CALL(test_3c);
	TEST_SAFE_CALL(test_3d);
	TEST_SAFE_CALL(test_3e);
	TEST_SAFE_CALL(test_3f);
	TEST_SAFE_CALL(test_3g);
}
