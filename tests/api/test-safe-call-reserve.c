/*
 *  duk_safe_call() reserve handling
 */

/*===
*** test_1 (duk_pcall)
top before: 8
top in dummy_func: 8
duk_safe_call() rc: 0
final top: 4101
==> rc=0, result='undefined'
*** test_2 (duk_pcall)
top before: 8
==> rc=1, result='TypeError: invalid args'
===*/

static duk_ret_t dummy_func(duk_context *ctx, void *udata) {
	(void) udata;
	printf("top in dummy_func: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_1(duk_context *ctx) {
	duk_int_t rc;

	/* Some ignored values. */
	duk_push_null(ctx);
	duk_push_null(ctx);
	duk_push_null(ctx);
	duk_push_null(ctx);
	duk_push_null(ctx);

	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "bar");
	duk_push_string(ctx, "quux");

	duk_require_stack(ctx, 4096 - 3);  /* 4096 nrets values */

	printf("top before: %ld\n", (long) duk_get_top(ctx));

	rc = duk_safe_call(ctx, dummy_func, NULL, 3 /*nargs*/, 4096 /*nrets*/);
	printf("duk_safe_call() rc: %ld\n", (long) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	duk_int_t rc;

	/* Some ignored values. */
	duk_push_null(ctx);
	duk_push_null(ctx);
	duk_push_null(ctx);
	duk_push_null(ctx);
	duk_push_null(ctx);

	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "bar");
	duk_push_string(ctx, "quux");

	duk_require_stack(ctx, 4096 - 3);  /* 4096 nrets values */

	printf("top before: %ld\n", (long) duk_get_top(ctx));

	/* This safe call tries to use 'nrets' too large for the current
	 * value stack reserve.  The reserve check is not exact and doesn't
	 * differentiate internal and user reserve (managing the user reserve
	 * is always the responsibility of calling code).
	 *
	 * The call fails with a throw (at least currently) rather than an
	 * error return value and a value at stack top; it may not always be
	 * possible to push an error object if there's no reserve at all.
	 *
	 * Internal reserve in Duktape 2.2: 32 entries, public reserve on
	 * Duktape/C function entry (and empty call stack) is 64 entries.
	 */
	rc = duk_safe_call(ctx, dummy_func, NULL, 3 /*nargs*/, 4096 + 32 + 8 /*nrets*/);
	printf("duk_safe_call() rc: %ld\n", (long) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	/* Use duk_pcall() so that value stack reserve is not extended
	 * for the top level activation.
	 */
	TEST_PCALL(test_1);
	TEST_PCALL(test_2);
}
