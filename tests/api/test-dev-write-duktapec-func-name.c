/*===
*** test_basic (duk_safe_call)
==> rc=1, result='TypeError: setter undefined'
===*/

static duk_ret_t dummy_func(duk_context *ctx) {
	(void) ctx;
	return 0;
}

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	/* User Duktape/C function .name is inherited from
	 * %NativeFunctionPrototype% and is not writable so
	 * a direct write fails.  This matches ES2015 .name
	 * property of ECMAScript functions which is non-writable
	 * but configurable.
	 */
	duk_push_c_function(ctx, dummy_func, 3);
	duk_push_string(ctx, "dummyFunc");
	duk_put_prop_string(ctx, -2, "name");
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
