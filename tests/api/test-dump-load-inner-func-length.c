/*
 *  Repro for https://github.com/svaarala/duktape/issues/1513
 */

/*===
*** test_1 (duk_safe_call)
outer.length in outer: 3
inner.length in outer: 4
outer.length in outer: 3
inner.length in outer: 4
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	duk_eval_string(ctx,
		"(function outer(a,b,c) {\n"
		"    function inner(x,y,z,w) {\n"
		"        print('outer.length in inner:', outer.length);\n"
		"        print('inner.length in inner:', inner.length);\n"
		"    }\n"
		"    print('outer.length in outer:', outer.length);\n"
		"    print('inner.length in outer:', inner.length);\n"
		"});\n");

	duk_dup_top(ctx);

	duk_call(ctx, 0);
	duk_pop(ctx);

	duk_dump_function(ctx);
	duk_load_function(ctx);

	duk_call(ctx, 0);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
