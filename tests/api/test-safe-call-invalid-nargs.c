/*
 *  duk_safe_call() with invalid nargs
 */

/*===
*** test_nargs_too_large (duk_safe_call)
top before: 3
==> rc=1, result='TypeError: invalid args'
*** test_nargs_minus1 (duk_safe_call)
top before: 3
==> rc=1, result='TypeError: invalid args'
===*/

static duk_ret_t dummy(duk_context *ctx, void *udata) {
	(void) ctx;
	(void) udata;
	return 0;
}

static duk_ret_t test_nargs_too_large(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	duk_push_null(ctx);
	duk_push_null(ctx);
	duk_push_null(ctx);
	printf("top before: %ld\n", (long) duk_get_top(ctx));

	rc = duk_safe_call(ctx, dummy, NULL, 4 /*nargs, too large*/, 0 /*nrets*/);
	printf("duk_safe_call rc: %ld\n", (long) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_nargs_minus1(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	duk_push_null(ctx);
	duk_push_null(ctx);
	duk_push_null(ctx);
	printf("top before: %ld\n", (long) duk_get_top(ctx));

	rc = duk_safe_call(ctx, dummy, NULL, -1 /*nargs, negative*/, 0 /*nrets*/);
	printf("duk_safe_call rc: %ld\n", (long) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_nargs_too_large);
	TEST_SAFE_CALL(test_nargs_minus1);
}
