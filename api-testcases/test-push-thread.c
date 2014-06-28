/*===
*** test_1 (duk_safe_call)
duk_is_object(1) = 1
duk_is_thread(1) = 1
top=2
concat: foobarquux
done
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
top: 2
context a: bar
context b: undefined
==> rc=0, result='undefined'
===*/

/* Some basic tests. */
static duk_ret_t test_1(duk_context *ctx) {
	duk_idx_t thr_idx;
	duk_context *new_ctx;

	duk_push_int(ctx, 123);  /* dummy */

	thr_idx = duk_push_thread(ctx);
	printf("duk_is_object(%ld) = %d\n", (long) thr_idx, (int) duk_is_object(ctx, thr_idx));
	printf("duk_is_thread(%ld) = %d\n", (long) thr_idx, (int) duk_is_thread(ctx, thr_idx));
	printf("top=%ld\n", (long) duk_get_top(ctx));

	/* use the thread (context) value stack */
	new_ctx = duk_get_context(ctx, thr_idx);
	duk_push_string(new_ctx, "foo");
	duk_push_string(new_ctx, "bar");
	duk_push_string(new_ctx, "quux");
	duk_concat(new_ctx, 3);
	printf("concat: %s\n", duk_get_string(new_ctx, -1));

	/* make new thread unreachable, so it gets GC'd */
	duk_set_top(ctx, 0);
	printf("done\n");
	return 0;
}

/* Thread with shared and fresh globals. */
static int test_2(duk_context *ctx) {
	duk_context *ctx_a;
	duk_context *ctx_b;

	duk_eval_string_noresult(ctx, "this.globalFoo = 'bar';");

	duk_push_thread(ctx);
	ctx_a = duk_require_context(ctx, -1);
	duk_push_thread_new_globalenv(ctx);
	ctx_b = duk_require_context(ctx, -1);

	printf("top: %ld\n", (long) duk_get_top(ctx));

	/* index 0: thread with globals shared with 'ctx' (has globalFoo)
	 * index 1: thread with globals separate with 'ctx'
	 */

	/* Print globalFoo. */

	duk_peval_string_noresult(ctx_a, "print('context a: ' + String(this.globalFoo));");
	duk_peval_string_noresult(ctx_b, "print('context b: ' + String(this.globalFoo));");

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
}
