/*===
*** test_1 (duk_safe_call)
0: 123
1: 345
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
remove at 2 ok
remove at -1 ok
==> rc=1, result='Error: invalid stack index 1'
*** test_3 (duk_safe_call)
remove at 0 ok
remove at -2 ok
==> rc=1, result='Error: invalid stack index -2'
*** test_4 (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
===*/

static void prep(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_push_int(ctx, 345);       /* -> [ 123 234 345 ] */
}

static duk_ret_t test_1(duk_context *ctx) {
	duk_idx_t i, n;

	prep(ctx);
	duk_remove(ctx, -2);          /* -> [ 123 345 ] */

	n = duk_get_top(ctx);
	for (i = 0; i < n; i++) {
		printf("%ld: %s\n", (long) i, duk_to_string(ctx, i));
	}
	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	prep(ctx);
	duk_remove(ctx, 2);   /* -> [ 123 234 ]  (legal) */
	printf("remove at 2 ok\n");
	duk_remove(ctx, -1);  /* -> [ 123 ]  (legal) */
	printf("remove at -1 ok\n");
	duk_remove(ctx, 1);   /* (illegal) */
	printf("remove at 1 ok\n");
	return 0;
}

static duk_ret_t test_3(duk_context *ctx) {
	prep(ctx);
	duk_remove(ctx, 0);   /* -> [ 234 345 ]  (legal) */
	printf("remove at 0 ok\n");
	duk_remove(ctx, -2);  /* -> [ 345 ]  (legal) */
	printf("remove at -2 ok\n");
	duk_remove(ctx, -2);   /* (illegal) */
	printf("remove at -2 ok\n");
	return 0;
}

static duk_ret_t test_4(duk_context *ctx) {
	prep(ctx);
	duk_remove(ctx, DUK_INVALID_INDEX);  /* (illegal) */
	printf("remove at DUK_INVALID_INDEX ok\n");
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
}
