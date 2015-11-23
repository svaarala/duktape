/*===
*** test_1 (duk_safe_call)
number: 123.000000
number: nan
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
==> rc=1, result='TypeError: number required, found null (stack index 0)'
*** test_3 (duk_safe_call)
==> rc=1, result='TypeError: number required, found none (stack index 0)'
*** test_4 (duk_safe_call)
==> rc=1, result='TypeError: number required, found none (stack index -2147483648)'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_int(ctx, 123);
	duk_push_nan(ctx);
	printf("number: %lf\n", (double) duk_require_number(ctx, 0));
	printf("number: %lf\n", (double) duk_require_number(ctx, 1));
	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_null(ctx);
	printf("number: %lf\n", (double) duk_require_number(ctx, 0));
	return 0;
}

static duk_ret_t test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);
	printf("number: %lf\n", (double) duk_require_number(ctx, 0));
	return 0;
}

static duk_ret_t test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);
	printf("number: %lf\n", (double) duk_require_number(ctx, DUK_INVALID_INDEX));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
}
