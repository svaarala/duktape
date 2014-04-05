/*===
pointer: 0xdeadbeef
pointer: (nil)
rc=0, result=undefined
rc=1, result=TypeError: not pointer
rc=1, result=TypeError: not pointer
rc=1, result=TypeError: not pointer
===*/

int test_1(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_pointer(ctx, (void *) 0xdeadbeef);
	duk_push_pointer(ctx, (void *) NULL);
	printf("pointer: %p\n", duk_require_pointer(ctx, 0));
	printf("pointer: %p\n", duk_require_pointer(ctx, 1));
	return 0;
}

int test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_null(ctx);
	printf("pointer: %p\n", duk_require_pointer(ctx, 0));
	return 0;
}

int test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);
	printf("pointer: %p\n", duk_require_pointer(ctx, 0));
	return 0;
}

int test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);
	printf("pointer: %p\n", duk_require_pointer(ctx, DUK_INVALID_INDEX));
	return 0;
}

void test(duk_context *ctx) {
	int rc;

	rc = duk_safe_call(ctx, test_1, 0, 1);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_2, 0, 1);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_3, 0, 1);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_4, 0, 1);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);
}

