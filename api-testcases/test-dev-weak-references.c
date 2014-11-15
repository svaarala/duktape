/*
 *  Weak reference tests
 */

/*===
*** test_weak_basic (duk_safe_call)
before 1st gc
after 1st gc
before 2nd gc
finalizer called
after 2nd gc
type of weakRef: 6
before 3rd gc
after 3rd gc
type of weakRef: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_weak_basic(duk_context *ctx) {
	duk_push_object(ctx);
	duk_set_weak(ctx, -1);

	duk_eval_string_noresult(ctx,
		"MyFinalizer = function () { print('finalizer called'); }");

	duk_eval_string(ctx,
		"(function () {\n"
		"    var obj = {};\n"
		"    Duktape.fin(obj, MyFinalizer);\n"
		"    return obj;\n"
		"})()");
	duk_dup_top(ctx);

	/* [ weakObject obj obj ] */

	duk_put_global_string(ctx, "strongRef");

	/* [ weakObject obj ] */

	duk_put_prop_string(ctx, 0, "weakRef");

	/* [ weakObject ] */

	/* Force GC to see that 'obj' is still strongly reachable. */

	printf("before 1st gc\n");
	duk_gc(ctx, 0);
	printf("after 1st gc\n");

	/* Remove strong reference and run GC.  The finalizer will run on
	 * this GC round but the weak reference won't yet be removed.
	 */

	duk_eval_string_noresult(ctx, "strongRef = undefined;");
	printf("before 2nd gc\n");
	duk_gc(ctx, 0);
	printf("after 2nd gc\n");

	/* At this point the finalizer has been executed and the value will be
	 * swept on the next round.
	 *
	 * The value is still reachable here through weak references, despite
	 * the fact that it's finalizer has been executed.  Ideally it would
	 * exist but read back as undefined.
	 */

	duk_get_prop_string(ctx, 0, "weakRef");
	printf("type of weakRef: %ld\n", (long) duk_get_type(ctx, -1));
	duk_pop(ctx);

	/* Force GC for a third time.  Because the finalizer did not rescue
	 * the object, it will be swept on this round, and the weak reference
	 * will be deleted.
	 */

	printf("before 3rd gc\n");
	duk_gc(ctx, 0);
	printf("after 3rd gc\n");

	duk_get_prop_string(ctx, 0, "weakRef");
	printf("type of weakRef: %ld\n", (long) duk_get_type(ctx, -1));
	duk_pop(ctx);

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_weak_basic);
}
