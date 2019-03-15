/*===
*** test_1 (duk_safe_call)
duk_get_top[0] = '0'
duk_to_stacktrace[0] = 'Error
    at err (eval:1)
    at eval (eval:1) preventsyield'
duk_get_top[1] = '0'
duk_to_stacktrace[1] = '4'
duk_get_top[2] = '0'
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	printf("duk_get_top[0] = '%ld'\n", (long) duk_get_top(ctx));

	duk_peval_string(ctx, "function err() { throw new Error(); }; err();");
	printf("duk_to_stacktrace[0] = '%s'\n", duk_to_stacktrace(ctx, -1));
	duk_pop(ctx);

	printf("duk_get_top[1] = '%ld'\n", (long) duk_get_top(ctx));

	duk_peval_string(ctx, "2+2");
	printf("duk_to_stacktrace[1] = '%s'\n", duk_to_stacktrace(ctx, -1));
	duk_pop(ctx);

	printf("duk_get_top[2] = '%ld'\n", (long) duk_get_top(ctx));

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
