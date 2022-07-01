/*===
*** test_1 (duk_safe_call)
nan
nan
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) duk_push_fixed_buffer(ctx, 0);
	(void) duk_push_fixed_buffer(ctx, 16);

	printf("%lf\n", duk_to_number(ctx, 0));
	printf("%lf\n", duk_to_number(ctx, 1));

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
