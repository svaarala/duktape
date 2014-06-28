/*
 *  Test how error handler works when error is created/thrown from C code.
 */

/*===
*** test_1 (duk_safe_call)
==> rc=1, result='ForcedName: range error: 123'
*** test_2 (duk_safe_call)
==> rc=1, result='ForcedName: arbitrary error code'
*** test_3 (duk_safe_call)
==> rc=1, result='ReferenceError: identifier 'zork' undefined'
*** test_4 (duk_safe_call)
string coerced: ForcedName: range error: 123
final top: 0
==> rc=0, result='undefined'
*** test_5 (duk_safe_call)
==> rc=1, result='ForcedName: arbitrary error code'
===*/

static void remove_handlers(duk_context *ctx) {
	duk_eval_string(ctx, "delete Duktape.errCreate; delete Duktape.errThrow;");
	duk_pop(ctx);
}

static duk_ret_t test_1(duk_context *ctx) {
	duk_set_top(ctx, 0);

	remove_handlers(ctx);
	duk_eval_string(ctx, "Duktape.errThrow = function (err) { err.name = 'ForcedName'; return err; }");
	duk_pop(ctx);

	/* Throw with duk_throw(). */

	duk_push_error_object(ctx, DUK_ERR_RANGE_ERROR, "range error: %d", 123);
	duk_throw(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);

	remove_handlers(ctx);
	duk_eval_string(ctx, "Duktape.errThrow = function (err) { err.name = 'ForcedName'; return err; }");
	duk_pop(ctx);

	/* Throw with duk_error(). */

	duk_error(ctx, 1234567, "arbitrary error code");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);

	/* Causes a ReferenceError when error handler runs.  The
	 * ReferenceError replaces the original TypeError.
	 */

	remove_handlers(ctx);
	duk_eval_string(ctx, "Duktape.errThrow = function (err) { zork; }");
	duk_pop(ctx);

	duk_error(ctx, DUK_ERR_TYPE_ERROR, NULL);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);

	remove_handlers(ctx);
	duk_eval_string(ctx, "Duktape.errCreate = function (err) { err.name = 'ForcedName'; return err; }");
	duk_pop(ctx);

	/* Create without throwing. */

	duk_push_error_object(ctx, DUK_ERR_RANGE_ERROR, "range error: %d", 123);
	printf("string coerced: %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_5(duk_context *ctx) {
	duk_set_top(ctx, 0);

	remove_handlers(ctx);
	duk_eval_string(ctx, "Duktape.errCreate = function (err) { err.name = 'ForcedName'; return err; }");

	/* Create (and throw) with duk_error(). */

	duk_error(ctx, 1234567, "arbitrary error code");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
	TEST_SAFE_CALL(test_5);
}
