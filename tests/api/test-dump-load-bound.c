/*
 *  Bound functions and dump/load.
 */

/*===
*** test_1 (duk_safe_call)
dump
==> rc=1, result='TypeError: compiledfunction required, found [object Function] (stack index -1)'
===*/

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	duk_eval_string(ctx,
		"(function () {\n"
		"    var func = function foo(a,b,c) { print('foo', a, b, c); };\n"
		"    return func.bind(null, 1, 2);\n"
		"})()\n");

	printf("dump\n");
	duk_dump_function(ctx);

	printf("load\n");
	duk_load_function(ctx);

	printf("call\n");
	duk_call(ctx, 0);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
