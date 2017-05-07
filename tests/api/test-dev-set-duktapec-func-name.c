/*===
*** test_basic (duk_safe_call)
added own .name
function
false
false
true
myFunc
final top: 2
==> rc=0, result='undefined'
===*/

static duk_ret_t dummy_func(duk_context *ctx) {
	(void) ctx;
	return 0;
}

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_c_function(ctx, dummy_func, 1);
	duk_push_string(ctx, "name");
	duk_push_string(ctx, "myFunc");
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE |
	                      DUK_DEFPROP_CLEAR_WRITABLE |
	                      DUK_DEFPROP_CLEAR_ENUMERABLE |
	                      DUK_DEFPROP_SET_CONFIGURABLE);

	printf("added own .name\n");

	duk_eval_string(ctx,
		"(function (fn) {\n"
		"    print(typeof fn);\n"
		"    var desc = Object.getOwnPropertyDescriptor(fn, 'name');\n"
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
