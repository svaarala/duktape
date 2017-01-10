/*===
*** test_1 (duk_safe_call)
pointer: 0xdeadbeef
pointer: (nil)
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
==> rc=1, result='TypeError: pointer required, found null (stack index 0)'
*** test_3 (duk_safe_call)
==> rc=1, result='TypeError: pointer required, found none (stack index 0)'
*** test_4 (duk_safe_call)
==> rc=1, result='TypeError: pointer required, found none (stack index -2147483648)'
===*/

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	duk_set_top(ctx, 0);
	duk_push_pointer(ctx, (void *) 0xdeadbeefUL);
	duk_push_pointer(ctx, (void *) NULL);
	printf("pointer: %p\n", duk_require_pointer(ctx, 0));
	printf("pointer: %p\n", duk_require_pointer(ctx, 1));
	return 0;
}

static duk_ret_t test_2(duk_context *ctx, void *udata) {
	(void) udata;

	duk_set_top(ctx, 0);
	duk_push_null(ctx);
	printf("pointer: %p\n", duk_require_pointer(ctx, 0));
	return 0;
}

static duk_ret_t test_3(duk_context *ctx, void *udata) {
	(void) udata;

	duk_set_top(ctx, 0);
	printf("pointer: %p\n", duk_require_pointer(ctx, 0));
	return 0;
}

static duk_ret_t test_4(duk_context *ctx, void *udata) {
	(void) udata;

	duk_set_top(ctx, 0);
	printf("pointer: %p\n", duk_require_pointer(ctx, DUK_INVALID_INDEX));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
}
