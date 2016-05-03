/*
 *  Test finalizer rescue when the value being finalized/rescued becomes
 *  unreachable as part of call handling unwind.
 */

/*===
*** test_1 (duk_safe_call)
calling
finalizer, rescuing object {"name":"my object"}
returned
calling
finalizer, rescuing object {"name":"my object"}
returned
final top: 0
==> rc=0, result='undefined'
finalizer, rescuing object {"name":"my object"}
===*/

static duk_ret_t my_func(duk_context *ctx) {
	(void) ctx;
	return 0;
}

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	duk_eval_string_noresult(ctx,
		"this.myFinalizer = function myfin(o) {\n"
		"    print('finalizer, rescuing object', JSON.stringify(o));\n"
		"    var global = new Function('return this')();\n"
		"    global.rescued = o;\n"
		"}\n"
	);

	/* First call; object gets finalized when call returns and call
	 * handling does a duk_set_top() call.  The object is rescued and
	 * later reused.
	 */
	duk_push_c_function(ctx, my_func, 3 /*nargs*/);
	duk_push_null(ctx);
	duk_eval_string(ctx,
		"(function () {\n"
		"    var res = { name: 'my object' };\n"
		"    Duktape.fin(res, myFinalizer);\n"
		"    return res;\n"
		"})()\n");
	duk_push_null(ctx);
	printf("calling\n");
	duk_call(ctx, 3);
	printf("returned\n");
	duk_pop(ctx);

	/* Object is reused for another call, and rescued again. */
	duk_push_c_function(ctx, my_func, 3 /*nargs*/);
	duk_push_null(ctx);
	duk_eval_string(ctx, "rescued");
	duk_eval_string_noresult(ctx, "rescued = null;");
	duk_push_null(ctx);
	printf("calling\n");
	duk_call(ctx, 3);
	printf("returned\n");
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);

	/* When we return the object is finalized once more as part of
	 * heap destruction but no longer rescued.
	 */
}
