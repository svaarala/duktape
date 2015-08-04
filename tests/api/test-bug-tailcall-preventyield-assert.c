/*
 *  In Duktape 0.10.0 the following assert would fail in some cases (reported
 *  by Andreas Oman):
 *
 *    assertion failed: (act->flags & DUK_ACT_FLAG_PREVENT_YIELD) == 0 (duk_js_call.c:1914)
 *
 *  This happened when:
 *
 *    - A C function called an Ecmascript function
 *    - That Ecmascript function tailcalled another Ecmascript function
 *
 *  What happens under the hood:
 *
 *   - The first C-to-Ecmascript call establishes an activation which has
 *     DUK_ACT_FLAG_PREVENT_YIELD set.
 *
 *   - The Ecmascript-to-Ecmascript call is in principle an allowed tail
 *     call.  However, the current call handling code assumes that the
 *     reused activation cannot have DUK_ACT_FLAG_PREVENT_YIELD set.
 *
 *  The assert is not incorrect: if the assert is removed, the tailcall
 *  handling code will unwind the activation and then reuse it.  As a
 *  result, the reused activation will *NOT* have DUK_ACT_FLAG_PREVENT_YIELD
 *  set, so that a Duktape.Thread.yield() would incorrectly be allowed.
 *  Actually getting this to happen is very difficult because there are
 *  several state checks which prevent this from actually happening.
 *
 *  A simple fix is to convert the tailcall to a normal call if the
 *  current activation has incompatible flags.  This would then prevent
 *  a tailcall even in an Ecmascript-to-Ecmascript case if the current
 *  frame was called from C.
 *
 *  NOTE: test_1() only fails with asserts enabled.
 */

/*===
*** test_1 (duk_safe_call)
f
g
result: 123
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	/* This test would trigger the assertion failure in Duktape 0.10.0. */

	duk_eval_string(ctx, "function g() { print('g'); return 123; };\n"
	                     "function f() { print('f'); return g(); };\n"
	                     "f;");
	duk_call(ctx, 0);
	printf("result: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
