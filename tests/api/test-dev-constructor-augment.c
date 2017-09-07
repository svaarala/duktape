/*
 *  Error augmentation in duk_new().
 */

/*===
*** test_1 (duk_safe_call)
err.augmented: true
err._Tracedata exists: 1
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	duk_eval_string_noresult(ctx,
		"(function () {\n"
		"    Duktape.errCreate = function (v) { v.augmented = true; return v; };\n"
		"})()\n");

	duk_eval_string(ctx, "RangeError");
	duk_push_string(ctx, "aiee");
	duk_new(ctx, 1);

	duk_get_prop_string(ctx, -1, "augmented");
	printf("err.augmented: %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "\x82" "Tracedata");
	printf("err._Tracedata exists: %ld\n", (long) duk_is_object(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
