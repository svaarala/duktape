/*
 *  A Duktape/C function does not have an automatic name in Duktape 1.x.
 *  You can set it yourself in Duktape 1.1 to get nicer tracebacks.
 *  In Duktape 1.0 Function.prototype.name is not writable so you can't
 *  do this.
 */

/*===
*** test_without_name (duk_safe_call)
my name is: ''
URIError: uri error (rc -106)
	anon  native strict preventsyield
	forEach  native strict preventsyield
	eval XXX preventsyield
==> rc=0, result='undefined'
*** test_with_name (duk_safe_call)
my name is: 'my_func'
URIError: uri error (rc -106)
	my_func  native strict preventsyield
	forEach  native strict preventsyield
	eval XXX preventsyield
==> rc=0, result='undefined'
===*/

static duk_ret_t my_func(duk_context *ctx) {
	duk_push_current_function(ctx);
	duk_get_prop_string(ctx, -1, "name");
	printf("my name is: '%s'\n", duk_safe_to_string(ctx, -1));
	duk_pop_2(ctx);

	return DUK_RET_URI_ERROR;
}

static duk_ret_t test_without_name(duk_context *ctx) {
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

static duk_ret_t test_with_name(duk_context *ctx) {
	duk_get_global_string(ctx, "MyFunc");
	duk_push_string(ctx, "my_func");
	duk_put_prop_string(ctx, -2, "name");
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
