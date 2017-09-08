/*
 *  duk_instanceof() error message when right side argument has no .prototype
 *  property, which is common for Duktape/C constructor functions.
 */

/*===
*** test_ecma (duk_safe_call)
==> rc=1, result='TypeError: instanceof rval has no .prototype'
*** test_c (duk_safe_call)
==> rc=1, result='TypeError: instanceof rval has no .prototype'
===*/

static duk_ret_t test_ecma(duk_context *ctx) {
	duk_eval_string(ctx,
		"(function () {\n"
		"    function Constructor() {}\n"
		"    Constructor.prototype = void 0;\n"
		"    print({} instanceof Constructor);\n"
		"})()");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t my_constructor(duk_context *ctx) {
	(void) ctx;
	return 0;
}

static duk_ret_t test_c(duk_context *ctx) {
	duk_bool_t rc;

	duk_push_c_function(ctx, my_constructor, 0);
	duk_push_object(ctx);
	rc = duk_instanceof(ctx, -1, -2);  /* {} instanceof my_constructor */
	printf("duk_instanceof() -> rc=%ld\n", (long) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_ecma);
	TEST_SAFE_CALL(test_c);
}
