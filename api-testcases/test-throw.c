/*===
*** test_1 (duk_safe_call)
==> rc=1, result='throw me'
===*/

int test_1(duk_context *ctx) {
	duk_push_string(ctx, "throw me");
	duk_throw(ctx);
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
