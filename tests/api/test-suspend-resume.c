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
*** test_suspend_resume_throw_trivial (duk_safe_call)
==> rc=1, result='RangeError: aiee'
*** test_suspend_resume_throw_basic (duk_safe_call)
other thread executing
==> rc=1, result='RangeError: aiee'
*** test_suspend_resume_reterr_trivial (duk_safe_call)
==> rc=1, result='RangeError: error (rc -3)'
*** test_suspend_resume_reterr_basic (duk_safe_call)
other thread executing
==> rc=1, result='RangeError: error (rc -3)'
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

/* A few tests to exercise suspend/resume storing and restoring of longjmp
 * context.
 */

static duk_ret_t test_suspend_resume_throw_trivial(duk_context *ctx, void *udata) {
	duk_thread_state st;

	duk_suspend(ctx, &st);
	duk_resume(ctx, &st);
	(void) duk_range_error(ctx, "aiee");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_suspend_resume_throw_basic(duk_context *ctx, void *udata) {
	duk_context *other;
	duk_thread_state st;

	(void) duk_push_thread(ctx);
	other = duk_get_context(ctx, -1);

	duk_suspend(ctx, &st);
	duk_eval_string(other, "print('other thread executing');");
	duk_resume(ctx, &st);
	(void) duk_range_error(ctx, "aiee");

	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_suspend_resume_reterr_trivial(duk_context *ctx, void *udata) {
	duk_thread_state st;

	duk_suspend(ctx, &st);
	duk_resume(ctx, &st);
	return DUK_RET_RANGE_ERROR;
}

static duk_ret_t test_suspend_resume_reterr_basic(duk_context *ctx, void *udata) {
	duk_context *other;
	duk_thread_state st;

	(void) duk_push_thread(ctx);
	other = duk_get_context(ctx, -1);

	duk_suspend(ctx, &st);
	duk_eval_string(other, "try { throw 'other thread executing'; } catch (e) { print(e); }");
	duk_resume(ctx, &st);
	return DUK_RET_RANGE_ERROR;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_trivial);
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_suspend_resume_throw_trivial);
	TEST_SAFE_CALL(test_suspend_resume_throw_basic);
	TEST_SAFE_CALL(test_suspend_resume_reterr_trivial);
	TEST_SAFE_CALL(test_suspend_resume_reterr_basic);
}
