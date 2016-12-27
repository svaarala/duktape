/*
 *  Illustrate corner case string intern side effect behavior: when a finalizer
 *  is triggered during string interning it can potentially invalidate the
 *  string data pointer/length provided before the data has been copied.
 *
 *  This would be quite difficult to trigger in practice.  This tests case
 *  relies on voluntary mark-and-sweep to trigger when the new duk_hstring
 *  is allocated.  To achieve that a lot of tries are needed.
 *
 *  https://github.com/svaarala/duktape/pull/884
 *
 *  Run with valgrind to catch memory unsafe behavior.
 */

/* When testing manually, increase to e.g. 100. */
#define TEST_COUNT 4

/*===
*** test_side_effect (duk_safe_call)
...0
finalizer
resized
...1
finalizer
resized
...2
finalizer
resized
...3
finalizer
resized
final top: 0
==> rc=0, result='undefined'
===*/

static int finalizer_ran = 0;

static duk_ret_t my_finalizer(duk_context *ctx) {
	finalizer_ran = 1;
#if 1
	printf("finalizer\n"); fflush(stdout);
#endif
	duk_get_global_string(ctx, "currentBuffer");
	duk_resize_buffer(ctx, -1, 0);
#if 1
	printf("resized\n"); fflush(stdout);
#endif
	return 0;
}

static duk_ret_t test_side_effect(duk_context *ctx, void *udata) {
	int i, count_intern;
	void *ptr;

	(void) udata;

	for (i = 0; i < TEST_COUNT; i++) {
		printf("...%d\n", i); fflush(stdout);

		/* Create a dynamic buffer we tweak in the side effect.
		 * Buffer is not yet registered to global.currentBuffer
		 * so any previous finalizers (which might still exist)
		 * can't reach it.
		 */

		ptr = duk_push_dynamic_buffer(ctx, 1024 * 1024);

		/* Create cyclical garbage which won't get collected by
		 * refcounting.  This is essential so that there can be
		 * something with a finalizer when mark-and-sweep gets
		 * triggered.
		 */

		duk_push_object(ctx);
		duk_push_object(ctx);
		duk_dup(ctx, -2);
		duk_put_prop_string(ctx, -2, "ref");
		duk_dup(ctx, -1);
		duk_put_prop_string(ctx, -3, "ref");
		duk_push_c_function(ctx, my_finalizer, 0);
		duk_set_finalizer(ctx, -2);

		/* Fill the buffer and make it reachable for the finalizer. */

		finalizer_ran = 0;
		memset(ptr, 0, 1024 * 1024);
		*((int *) ptr) = i;  /* Create a new string on each round. */
		duk_dup(ctx, -3);
		duk_put_global_string(ctx, "currentBuffer");
		duk_remove(ctx, -3);

		/* We're now ready: cause the two objects to become not yet
		 * collected garbage.  The buffer may be resized before we
		 * reach the string test, so don't reference 'ptr' anymore.
		 */

		duk_pop_2(ctx);

		/* Finally, push a new string and hope we trigger the
		 * finalizer.  Repeat the operation until the finalizer
		 * runs (it may already have run), and hope it gets triggered
		 * in the right spot.
		 */

		count_intern = 0;
		while (!finalizer_ran) {
			count_intern++;
			duk_push_lstring(ctx, ptr, 1024 * 1024);
			duk_pop(ctx);

			/* Because the fix in https://github.com/svaarala/duktape/pull/884
			 * prevent side effects in string interning, we need to trigger
			 * them explicitly at some pointer to avoid looping forever.
			 * The limit here allows a failure to be detected in e.g. 1.5.0.
			 */
			if (count_intern >= 100000) {
				duk_gc(ctx, 0);
			}
		}
#if 0
		printf("Finalizer run took %d tries \n", count_intern); fflush(stdout);
#endif
	}

	duk_gc(ctx, 0);
	duk_gc(ctx, 0);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_side_effect);
}
