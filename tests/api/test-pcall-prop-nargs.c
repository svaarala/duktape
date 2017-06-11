/*
 *  duk_pcall_prop() with invalid nargs
 */

/*===
*** test_nargs_too_large (duk_safe_call)
top before: 4
==> rc=1, result='TypeError: invalid args'
*** test_nargs_negative_minus1 (duk_safe_call)
top before: 4
==> rc=1, result='TypeError: invalid args'
*** test_nargs_negative_minus2 (duk_safe_call)
top before: 4
==> rc=1, result='TypeError: invalid args'
===*/

static duk_ret_t test_nargs_too_large(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	duk_eval_string(ctx, "({ prop: function () { print('propcall'); } })");
	duk_push_string(ctx, "prop");  /* key + 2 args */
	duk_push_null(ctx);
	duk_push_null(ctx);

	printf("top before: %ld\n", (long) duk_get_top(ctx));

	/* NOTE: nargs == 3 isn't technically too large because the object
	 * will be used as the key too.
	 */
	rc = duk_pcall_prop(ctx, -4 /*obj idx*/, 4 /*nargs, too large*/);
	printf("duk_pcall rc: %ld\n", (long) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_nargs_negative_minus1(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	duk_eval_string(ctx, "({ prop: function () { print('propcall'); } })");
	duk_push_string(ctx, "prop");  /* key + 2 args */
	duk_push_null(ctx);
	duk_push_null(ctx);

	printf("top before: %ld\n", (long) duk_get_top(ctx));

	rc = duk_pcall_prop(ctx, -4 /*obj idx*/, -1 /*nargs, negative*/);
	printf("duk_pcall rc: %ld\n", (long) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_nargs_negative_minus2(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	duk_eval_string(ctx, "({ prop: function () { print('propcall'); } })");
	duk_push_string(ctx, "prop");  /* key + 2 args */
	duk_push_null(ctx);
	duk_push_null(ctx);

	printf("top before: %ld\n", (long) duk_get_top(ctx));

	rc = duk_pcall_prop(ctx, -4 /*obj idx*/, -2 /*nargs, negative*/);
	printf("duk_pcall rc: %ld\n", (long) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_nargs_too_large);
	TEST_SAFE_CALL(test_nargs_negative_minus1);
	TEST_SAFE_CALL(test_nargs_negative_minus2);
}
