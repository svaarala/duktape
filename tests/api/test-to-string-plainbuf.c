/*===
*** test_1 (duk_safe_call)
[object Uint8Array]
[object Uint8Array]
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) duk_push_fixed_buffer(ctx, 0);
	(void) duk_push_fixed_buffer(ctx, 16);

	printf("%s\n", duk_to_string(ctx, 0));
	printf("%s\n", duk_to_string(ctx, 1));

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
