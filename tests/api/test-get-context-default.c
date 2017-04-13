/*===
*** test_1 (duk_safe_call)
concat: foobarquux
still here
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
new_ctx is dummy: 1
new_ctx is dummy: 1
new_ctx is dummy: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	duk_context *new_ctx;

	(void) udata;

	duk_set_top(ctx, 0);
	(void) duk_push_thread(ctx);
	new_ctx = duk_get_context_default(ctx, -1, NULL);

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

static duk_ret_t test_2(duk_context *ctx, void *udata) {
	duk_context *new_ctx;
	char dummy;

	(void) udata;

	/* non-thread value */
	duk_set_top(ctx, 0);
	duk_push_int(ctx, 123);
	new_ctx = duk_get_context_default(ctx, -1, (duk_context *) &dummy);
	printf("new_ctx is dummy: %d\n", (new_ctx == (duk_context *) &dummy ? 1 : 0));

	/* invalid index */
	new_ctx = duk_get_context_default(ctx, 123, (duk_context *) &dummy);
	printf("new_ctx is dummy: %d\n", (new_ctx == (duk_context *) &dummy ? 1 : 0));

	/* invalid index */
	new_ctx = duk_get_context_default(ctx, DUK_INVALID_INDEX, (duk_context *) &dummy);
	printf("new_ctx is dummy: %d\n", (new_ctx == (duk_context *) &dummy ? 1 : 0));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
}
