/*===
*** test_1 (duk_safe_call)
concat: foobarquux
still here
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
new_ctx is NULL: 1
new_ctx is NULL: 1
new_ctx is NULL: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	duk_context *new_ctx;

	duk_set_top(ctx, 0);
	(void) duk_push_thread(ctx);
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
	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	duk_context *new_ctx;

	/* non-thread value */
	duk_set_top(ctx, 0);
	duk_push_int(ctx, 123);
	new_ctx = duk_get_context(ctx, -1);
	printf("new_ctx is NULL: %d\n", (new_ctx == NULL ? 1 : 0));

	/* invalid index */
	new_ctx = duk_get_context(ctx, 123);
	printf("new_ctx is NULL: %d\n", (new_ctx == NULL ? 1 : 0));

	/* invalid index */
	new_ctx = duk_get_context(ctx, DUK_INVALID_INDEX);
	printf("new_ctx is NULL: %d\n", (new_ctx == NULL ? 1 : 0));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
}
