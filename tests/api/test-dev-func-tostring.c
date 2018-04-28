/*
 *  Test the current Function .toString() format.
 *
 *  Cover a few cases which cannot be exercised using ECMAScript code alone.
 */

/*===
*** test_1 (duk_safe_call)
function light_PTR() { [lightfunc code] }
function dummy {() { [native code] }
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t dummy_func(duk_context *ctx) {
	(void) ctx;
	return 0;
}

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	/* Lightfunc, must sanitize the address for the expect string. */
	duk_eval_string(ctx,
		"(function (v) {\n"
		"    print(String(v).replace(/light_[0-9a-fA-f_]+/, 'light_PTR'));\n"
		"})");
	duk_push_c_lightfunc(ctx, dummy_func, 0 /*nargs*/, 0 /*length*/, 0 /*magic*/);
	duk_call(ctx, 1);
	duk_pop(ctx);

	/* Function with a .name containing invalid characters.
	 *
	 * This is not currently handled very well: the .toString() output
	 * uses the name as is, which can make the output technically
	 * incorrect because it won't parse as a function.  However, the
	 * only way to create such functions is from C code so this is a
	 * minor issue.
	 */

	duk_push_c_function(ctx, dummy_func, 0 /*nargs*/);
	duk_push_string(ctx, "name");
	duk_push_string(ctx, "dummy {");
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_FORCE);
	printf("%s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
