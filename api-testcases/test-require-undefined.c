/*===
*** test_1 (duk_safe_call)
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
==> rc=1, result='TypeError: not undefined'
*** test_3 (duk_safe_call)
==> rc=1, result='TypeError: not undefined'
*** test_4 (duk_safe_call)
==> rc=1, result='TypeError: not undefined'
===*/

int test_1(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_undefined(ctx);
	duk_require_undefined(ctx, 0);
	return 0;
}

int test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_null(ctx);
	duk_require_undefined(ctx, 0);
	return 0;
}

int test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_require_undefined(ctx, 0);
	printf("require 0 OK\n");
	return 0;
}

int test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_require_undefined(ctx, DUK_INVALID_INDEX);
	printf("require DUK_INVALID_INDEX OK\n");
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
}
