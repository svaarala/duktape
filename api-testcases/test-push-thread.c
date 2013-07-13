/*===
duk_is_object(1) = 1
duk_is_thread(1) = 1
top=2
concat: foobarquux
done
===*/

void test(duk_context *ctx) {
	int thr_idx;
	duk_context *new_ctx;

	duk_push_int(ctx, 123);  /* dummy */

	thr_idx = duk_push_new_thread(ctx);
	printf("duk_is_object(%d) = %d\n", thr_idx, duk_is_object(ctx, thr_idx));
	printf("duk_is_thread(%d) = %d\n", thr_idx, duk_is_thread(ctx, thr_idx));
	printf("top=%d\n", duk_get_top(ctx));

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
}
