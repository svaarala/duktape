/*
 *  duk_pnew() invalid input stack
 */

/*===
*** test_nargs_too_large (duk_safe_call)
top before: 2
==> rc=1, result='TypeError: invalid args'
*** test_nargs_negative_minus1 (duk_safe_call)
top before: 3
==> rc=1, result='TypeError: invalid args'
===*/

duk_ret_t test_nargs_too_large(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "bar");

	printf("top before: %ld\n", (long) duk_get_top(ctx));

	rc = duk_pnew(ctx, 2);  /* not enough value stack args */
	printf("duk_pnew() rc: %ld\n", (long) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

duk_ret_t test_nargs_negative_minus1(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	duk_eval_string(ctx, "(function () {})");
	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "bar");

	printf("top before: %ld\n", (long) duk_get_top(ctx));

	rc = duk_pnew(ctx, -1);  /* negative nargs */
	printf("duk_pnew() rc: %ld\n", (long) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_nargs_too_large);
	TEST_SAFE_CALL(test_nargs_negative_minus1);
}
