/*===
concat: foobarquux
still here
===*/

void test(duk_context *ctx) {
	duk_context *new_ctx;

	(void) duk_push_new_thread(ctx);
	new_ctx = duk_get_context(ctx, -1);

	duk_push_string(new_ctx, "foo");
	duk_push_string(new_ctx, "bar");
	duk_push_string(new_ctx, "quux");
	duk_concat(new_ctx, 3);
	printf("concat: %s\n", duk_get_string(new_ctx, -1));

	/* This duk_pop() makes the new thread unreachable (assuming there
	 * is no other reference to it), so new_ctx is no longer valid
	 * afterwards.
	 */
	duk_pop(ctx);

	printf("still here\n");
}

