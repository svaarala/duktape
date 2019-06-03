/*===
*** test_1 (duk_safe_call)
func called
==> rc=1, result='TypeError: constructor requires 'new''
*** test_2 (duk_safe_call)
func called
still here
final top: 1
==> rc=0, result='undefined'
*** test_3 (duk_safe_call)
==> rc=1, result='TypeError: constructor requires 'new''
===*/

static duk_ret_t func(duk_context *ctx) {
	printf("func called\n");
	duk_require_constructor_call(ctx);
	printf("still here\n");
	return 0;
}

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_c_function(ctx, func, 0);
	duk_call(ctx, 0);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_2(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_c_function(ctx, func, 0);
	duk_new(ctx, 0);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_3(duk_context *ctx, void *udata) {
	(void) udata;

	/* No current activation, also causes error. */
	duk_require_constructor_call(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
}
