/*===
*** test_1 (duk_safe_call)
boolean: 1
boolean: 0
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
==> rc=1, result='TypeError: boolean required, found null (stack index 0)'
*** test_3 (duk_safe_call)
==> rc=1, result='TypeError: boolean required, found none (stack index 0)'
*** test_4 (duk_safe_call)
==> rc=1, result='TypeError: boolean required, found none (stack index -2147483648)'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_true(ctx);
	duk_push_false(ctx);
	printf("boolean: %d\n", (int) duk_require_boolean(ctx, 0));
	printf("boolean: %d\n", (int) duk_require_boolean(ctx, 1));
	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_null(ctx);
	printf("boolean: %d\n", (int) duk_require_boolean(ctx, 0));
	return 0;
}

static duk_ret_t test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);
	printf("boolean: %d\n", (int) duk_require_boolean(ctx, 0));
	return 0;
}

static duk_ret_t test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);
	printf("boolean: %d\n", (int) duk_require_boolean(ctx, DUK_INVALID_INDEX));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
}
