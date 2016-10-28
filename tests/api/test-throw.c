/*===
*** test_basic (duk_safe_call)
==> rc=1, result='throw me'
*** test_return (duk_safe_call)
==> rc=1, result='throw me too'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_string(ctx, "throw me");
	duk_throw(ctx);
	return 0;
}

static duk_ret_t test_return(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_string(ctx, "throw me too");
	return duk_throw(ctx);
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_return);
}
