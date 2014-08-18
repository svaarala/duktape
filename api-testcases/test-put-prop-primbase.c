/*===
*** test_put (duk_safe_call)
==> rc=1, result='TypeError: invalid base value'
*** test_put (duk_pcall)
==> rc=1, result='TypeError: invalid base value'
===*/

duk_ret_t test_put(duk_context *ctx) {
	duk_ret_t rc;

	/* In Ecmascript, '(0).foo = "bar"' should work and evaluate to "bar"
	 * in non-strict mode, but cause an error to be thrown in strict mode
	 * (E5.1, Section 8.7.2, exotic [[Put]] variant, step 7.
	 */

	duk_push_int(ctx, 0);
	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "bar");
	rc = duk_put_prop(ctx, -3);

	printf("put rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	/* Since Duktape 0.12.0, a Duktape/C context is considered strict
	 * both inside and outside of Duktape/C calls.
	 */
	TEST_SAFE_CALL(test_put);  /* outside: strict (non-strict before 0.12.0) */
	TEST_PCALL(test_put);      /* inside: strict */
}
