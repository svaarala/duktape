/*
 *  Prototype loop is tricky to handle internally and must not cause e.g.
 *  GC failures.  Exercise a few common paths.
 */

/*===
*** test_1 (duk_safe_call)
first gc
make unreachable
second gc
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	duk_push_object(ctx);
	duk_push_object(ctx);
	duk_dup(ctx, 0);
	duk_set_prototype(ctx, 1);
	duk_dup(ctx, 1);
	duk_set_prototype(ctx, 0);

	/* Both objects are now in a prototype loop.  Force garbage
	 * collection to ensure nothing breaks.
	 */

	printf("first gc\n"); fflush(stdout);
	duk_gc(ctx, 0);

	/* Make the objects unreachable and re-run GC.  This triggers
	 * e.g. finalizer checks.
	 */

	printf("make unreachable\n"); fflush(stdout);
	duk_set_top(ctx, 0);

	printf("second gc\n"); fflush(stdout);
	duk_gc(ctx, 0);

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
