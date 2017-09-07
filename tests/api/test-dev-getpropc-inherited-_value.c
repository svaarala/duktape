/*===
*** test_1 (duk_safe_call)
==> rc=1, result='TypeError: [object Error] not callable'
===*/

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	/* Inherit _Value from Object.prototype. */
	duk_eval_string(ctx, "Object.prototype");
	duk_push_true(ctx);
	duk_put_prop_string(ctx, -2, "\x82" "Value");
	duk_pop(ctx);

	/* Create an Error without _Value and try to call it.  Call handling
	 * should indicate an error about a non-callable target rather than
	 * throwing the error object as is, as would be the case if _Value
	 * was an own property of the error (used by GETPROPC opcode).
	 */
	duk_eval_string(ctx, "new RangeError('aiee');");
	duk_call(ctx, 0);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
