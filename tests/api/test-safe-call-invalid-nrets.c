/*
 *  duk_safe_call() with invalid nrets
 */

/*===
*** test_nrets_too_large (duk_safe_call)
top before: 3
==> rc=1, result='TypeError: invalid args'
*** test_nrets_too_large_fixed (duk_safe_call)
top before: 3
duk_safe_call rc: 0
final top: 1025
==> rc=0, result='undefined'
*** test_nrets_minus1 (duk_safe_call)
top before: 3
==> rc=1, result='TypeError: invalid args'
===*/

static duk_ret_t dummy(duk_context *ctx, void *udata) {
	(void) ctx;
	(void) udata;
	return 0;
}

static duk_ret_t test_nrets_too_large(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	duk_push_null(ctx);
	duk_push_null(ctx);
	duk_push_null(ctx);
	printf("top before: %ld\n", (long) duk_get_top(ctx));

	/* Here 'nrets' is too large, beyond the current value stack
	 * reserve.  (This succeeds in Duktape 2.1 and prior because
	 * the implementation did a reserve check.)
	 */
	rc = duk_safe_call(ctx, dummy, NULL, 2 /*nargs*/, 1024 /*nrets, too large*/);
	printf("duk_safe_call rc: %ld\n", (long) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_nrets_too_large_fixed(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	duk_push_null(ctx);
	duk_push_null(ctx);
	duk_push_null(ctx);
	printf("top before: %ld\n", (long) duk_get_top(ctx));

	/* Here duk_check_stack() is used to extend the value stack
	 * reserve to accommodate 'nrets'.  The required reserve is:
	 *     current_top - nargs + nrets
	 * which may be negative if nargs >= nrets.
	 */
	rc = duk_check_stack(ctx, duk_get_top(ctx) - 2 + 1024);
	rc = duk_safe_call(ctx, dummy, NULL, 2 /*nargs*/, 1024 /*nrets, too large*/);
	printf("duk_safe_call rc: %ld\n", (long) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
static duk_ret_t test_nrets_minus1(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	duk_push_null(ctx);
	duk_push_null(ctx);
	duk_push_null(ctx);
	printf("top before: %ld\n", (long) duk_get_top(ctx));

	rc = duk_safe_call(ctx, dummy, NULL, 2 /*nargs*/, -1 /*nrets, negative*/);
	printf("duk_safe_call rc: %ld\n", (long) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_nrets_too_large);
	TEST_SAFE_CALL(test_nrets_too_large_fixed);
	TEST_SAFE_CALL(test_nrets_minus1);
}
