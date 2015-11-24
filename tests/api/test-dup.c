/*===
*** test_1 (duk_safe_call)
0: 123
1: 234
2: 123
3: 123
final top: 4
==> rc=0, result='undefined'
*** test_2a (duk_safe_call)
==> rc=1, result='Error: invalid stack index -3'
*** test_2b (duk_safe_call)
==> rc=1, result='Error: invalid stack index 2'
*** test_2c (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
*** test_3a (duk_safe_call)
==> rc=1, result='Error: invalid stack index -1'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	duk_idx_t i, n;

	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_dup(ctx, -2);  /* -> [ 123 234 123 ] */
	duk_dup_top(ctx);  /* -> [ 123 234 123 123 ] */

	n = duk_get_top(ctx);
	for (i = 0; i < n; i++) {
		printf("%ld: %s\n", (long) i, duk_to_string(ctx, i));
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_2a(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_dup(ctx, -3);  /* out of bounds */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_2b(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_dup(ctx, 2);  /* out of bounds */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_2c(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_dup(ctx, DUK_INVALID_INDEX);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_3a(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_dup_top(ctx);  /* empty */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2a);
	TEST_SAFE_CALL(test_2b);
	TEST_SAFE_CALL(test_2c);
	TEST_SAFE_CALL(test_3a);
}
