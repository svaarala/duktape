/*
 *  Repro for https://github.com/svaarala/duktape/pull/2282
 */

/*===
*** test_basic (duk_safe_call)
other thread executing
finalizer 2 called!
forcing gc
finalizer 1 called!
forced gc done, now resume
main thread back
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	duk_context *other;
	duk_thread_state st;

	(void) duk_push_thread(ctx);
	other = duk_get_context(ctx, -1);

	duk_eval_string_noresult(ctx,
		"var myObject1 = (function () {\n"
		"    var res = {};\n"
		"    Duktape.fin(res, function () { print('finalizer 1 called!'); });\n"
		"    return res;\n"
		"})()\n");
	duk_eval_string_noresult(ctx,
		"function myObject2Fin() { print('finalizer 2 called!'); }\n"
		"var myObject2 = (function () {\n"
		"    var res = {};\n"
		"    Duktape.fin(res, myObject2Fin);\n"
		"    res.prototype = null;\n"
		"    return res;\n"
		"})()\n");

	duk_suspend(ctx, &st);

	duk_eval_string_noresult(other, "print('other thread executing');");
	duk_eval_string_noresult(other, "myObject1 = null;");
	duk_eval_string_noresult(other, "myObject2 = null;");
	duk_eval_string_noresult(other, "print('forcing gc');");
	duk_eval_string_noresult(other, "Duktape.gc(); Duktape.gc(); Duktape.gc();");
	duk_eval_string_noresult(other, "print('forced gc done, now resume');");

	duk_resume(ctx, &st);

	duk_eval_string_noresult(ctx, "print('main thread back');");

	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
