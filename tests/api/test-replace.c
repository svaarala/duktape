/*===
*** test_1 (duk_safe_call)
0: 123
1: foo
2: 345
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
replace at 3 ok
==> rc=1, result='Error: invalid stack index 3'
*** test_3 (duk_safe_call)
replace at -4 ok
==> rc=1, result='Error: invalid stack index -4'
*** test_4 (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
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
	duk_replace(ctx, -3);         /* -> [ 123 "foo" 345 ] */

	n = duk_get_top(ctx);
	for (i = 0; i < n; i++) {
		printf("%ld: %s\n", (long) i, duk_to_string(ctx, i));
	}
	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	prep(ctx);
	duk_replace(ctx, 3);          /* -> [ 123 234 "foo" ]  (legal) */
	printf("replace at 3 ok\n");
	duk_replace(ctx, 3);          /* (illegal: index too high) */
	printf("replace at 3 ok\n");
	return 0;
}

static duk_ret_t test_3(duk_context *ctx) {
	prep(ctx);
	duk_replace(ctx, -4);         /* -> [ "foo" 234 345 ]  (legal) */
	printf("replace at -4 ok\n");
	duk_replace(ctx, -4);         /* (illegal: index too low) */
	printf("replace at -4 ok\n");
	return 0;
}

static duk_ret_t test_4(duk_context *ctx) {
	prep(ctx);
	duk_replace(ctx, DUK_INVALID_INDEX);  /* (illegal: invalid index) */
	printf("replace at DUK_INVALID_INDEX ok\n");
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
}
