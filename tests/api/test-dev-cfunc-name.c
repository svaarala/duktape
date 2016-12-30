/*
 *  A Duktape/C function does not have an automatic name in Duktape 1.x.
 *  You can set it yourself in Duktape 1.1 to get nicer tracebacks.
 *  In Duktape 1.0 Function.prototype.name is not writable so you can't
 *  do this.
 *
 *  In Duktape 2.x .name is non-writable but configurable to match ES2015
 *  requirements; duk_def_prop() can be used to set it.
 */

/*===
*** test_without_name (duk_safe_call)
my name is: ''
URIError: error (rc -7)
    at [anon] () native strict preventsyield
    at forEach () native strict preventsyield
    at eval XXX preventsyield
==> rc=0, result='undefined'
*** test_with_name (duk_safe_call)
my name is: 'my_func'
URIError: error (rc -7)
    at my_func () native strict preventsyield
    at forEach () native strict preventsyield
    at eval XXX preventsyield
==> rc=0, result='undefined'
===*/

static duk_ret_t my_func(duk_context *ctx) {
	duk_push_current_function(ctx);
	duk_get_prop_string(ctx, -1, "name");
	printf("my name is: '%s'\n", duk_safe_to_string(ctx, -1));
	duk_pop_2(ctx);

	return DUK_RET_URI_ERROR;
}

static duk_ret_t test_without_name(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_c_function(ctx, my_func, 0);
	duk_put_global_string(ctx, "MyFunc");

	duk_eval_string_noresult(ctx,
		"try {\n"
		"    [1].forEach(MyFunc);\n"
		"} catch (e) {\n"
		"    print(sanitize(e.stack || e));\n"
		"}\n");

	return 0;
}

static duk_ret_t test_with_name(duk_context *ctx, void *udata) {
	(void) udata;

	duk_get_global_string(ctx, "MyFunc");
	duk_push_string(ctx, "name");
	duk_push_string(ctx, "my_func");
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE);
	duk_pop(ctx);

	duk_eval_string_noresult(ctx,
		"try {\n"
		"    [1].forEach(MyFunc);\n"
		"} catch (e) {\n"
		"    print(sanitize(e.stack || e));\n"
		"}\n");

	return 0;
}

void test(duk_context *ctx) {
	duk_eval_string_noresult(ctx,
		"var sanitize = function(v) {\n"
		"    v = v.replace(/eval \\S+/, 'eval XXX');\n"
		"    return v;\n"
		"}\n");

	TEST_SAFE_CALL(test_without_name);
	TEST_SAFE_CALL(test_with_name);
}
