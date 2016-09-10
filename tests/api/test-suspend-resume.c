/*
 *  duk_suspend() and duk_resume().
 *
 *  This test avoids actual native multithreading on purpose.
 */

/*===
*** test_trivial (duk_safe_call)
final top: 0
==> rc=0, result='undefined'
*** test_basic (duk_safe_call)
other thread executing
final top: 0
==> rc=0, result='undefined'
===*/

/* Simplest possible case: suspend and resume immediately. */
static duk_ret_t test_trivial(duk_context *ctx, void *udata) {
	duk_thread_state st;

	duk_suspend(ctx, &st);
	duk_resume(ctx, &st);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Simple case where a single native thread is used; once we suspend, call into
 * Duktape using another thread.  That call finished completely (rather than
 * suspending) before we resume the original thread.
 */

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	duk_context *other;
	duk_thread_state st;

	(void) duk_push_thread(ctx);
	other = duk_get_context(ctx, -1);

	duk_suspend(ctx, &st);

	duk_eval_string(other, "print('other thread executing');");

	duk_resume(ctx, &st);

	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_trivial);
	TEST_SAFE_CALL(test_basic);
}
