/*===
===*/

static duk_ret_t dummy_func(duk_context *ctx) {
	(void) ctx;
	return 0;
}

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_c_function(ctx, dummy_func, 1);
	duk_push_string(ctx, "length");
	duk_push_int(ctx, 3);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE |
	                      DUK_DEFPROP_CLEAR_WRITABLE |
	                      DUK_DEFPROP_CLEAR_ENUMERABLE |
	                      DUK_DEFPROP_SET_CONFIGURABLE |
	                      DUK_DEFPROP_FORCE);

	printf("added concrete .length\n");

	duk_eval_string(ctx,
		"(function (fn) {\n"
		"    print(typeof fn);\n"
		"    var desc = Object.getOwnPropertyDescriptor(fn, 'length');\n"
		"    print(desc.writable);\n"
		"    print(desc.enumerable);\n"
		"    print(desc.configurable);\n"
		"    print(desc.value);\n"
		"})");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
