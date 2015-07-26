/*
 *  When 'ctx' has a catch point (safe call) and the safe call runs eval()
 *  in a different thread which throws an error, there is valgrind whine
 *  about interrupt_counter handling.  Other than the valgrind whine (which
 *  is quite harmless) the test case passes.
 *
 *  This happens both when pushing a thread with reused globals and fresh
 *  independent globals.
 */

/*===
*** test_1 (duk_safe_call)
top: 1
==> rc=1, result='ReferenceError: identifier 'aiee' undefined'
*** test_2 (duk_safe_call)
top: 1
==> rc=1, result='ReferenceError: identifier 'zork' undefined'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	duk_context *ctx2;

	duk_push_thread(ctx);
	ctx2 = duk_require_context(ctx, -1);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	duk_eval_string_noresult(ctx2, "aiee;");  /* ReferenceError */

	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	duk_context *ctx2;

	duk_push_thread_new_globalenv(ctx);
	ctx2 = duk_require_context(ctx, -1);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	duk_eval_string_noresult(ctx2, "zork;");  /* ReferenceError */

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
}
