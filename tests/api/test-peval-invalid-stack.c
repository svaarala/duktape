/*
 *  duk_peval() invalid input stack
 */

/*===
*** test_top_0 (duk_safe_call)
top before: 0
==> rc=1, result='TypeError: invalid args'
===*/

duk_ret_t test_top_0(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	printf("top before: %ld\n", (long) duk_get_top(ctx));

	rc = duk_peval(ctx);
	printf("duk_peval() rc: %ld\n", (long) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_top_0);
}
