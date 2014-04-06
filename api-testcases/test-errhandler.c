/*
 *  Test how error handler works when error is thrown from C code.
 */

/*===
*** test_1 (duk_safe_call)
==> rc=1, result='ForcedName: range error: 123'
*** test_2 (duk_safe_call)
==> rc=1, result='ForcedName: arbitrary error code'
*** test_3 (duk_safe_call)
==> rc=1, result='ReferenceError: identifier 'zork' undefined'
===*/

int test_1(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_eval_string(ctx, "Duktape.errhnd = function (err) { err.name = 'ForcedName'; return err; }");

	/* Throw with duk_throw(). */

	duk_push_error_object(ctx, DUK_ERR_RANGE_ERROR, "range error: %d", 123);
	duk_throw(ctx);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

int test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_eval_string(ctx, "Duktape.errhnd = function (err) { err.name = 'ForcedName'; return err; }");

	/* Throw with duk_error(). */

	duk_error(ctx, 1234567, "arbitrary error code");

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

int test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);

	/* Causes a ReferenceError when error handler runs.  The
	 * ReferenceError replaces the original TypeError.
	 */

	duk_eval_string(ctx, "Duktape.errhnd = function (err) { zork; }");

	duk_error(ctx, DUK_ERR_TYPE_ERROR, NULL);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
}

