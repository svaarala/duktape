/*===
buffer: ptr-is-NULL=0, sz=1024
buffer
buffer: ptr-is-NULL=-1, sz=0
buffer
buffer: ptr-is-NULL=0, sz=1024
buffer
buffer: ptr-is-NULL=-1, sz=0
buffer
rc=0, result=undefined
rc=1, result=TypeError: not buffer
rc=1, result=TypeError: not buffer
rc=1, result=TypeError: not buffer
===*/

int test_1(duk_context *ctx) {
	void *ptr;
	size_t sz;
	int i;

	duk_set_top(ctx, 0);
	duk_push_fixed_buffer(ctx, 1024);
	duk_push_fixed_buffer(ctx, 0);
	duk_push_dynamic_buffer(ctx, 1024);
	duk_push_dynamic_buffer(ctx, 0);

	for (i = 0; i < 4; i++) {
		sz = (size_t) 0xdeadbeef;
		ptr = duk_require_buffer(ctx, i, &sz);
		printf("buffer: ptr-is-NULL=%d, sz=%d\n", (sz == 0 ? -1 : (ptr == NULL ? 1 : 0)), (int) sz);

		sz = (size_t) 0xdeadbeef;
		ptr = duk_require_buffer(ctx, i, NULL);
		printf("buffer\n");
	}

	return 0;
}

int test_2(duk_context *ctx) {
	void *ptr;
	size_t sz;

	duk_set_top(ctx, 0);
	duk_push_null(ctx);
	sz = (size_t) 0xdeadbeef;
	ptr = duk_require_buffer(ctx, 0, &sz);
	printf("buffer ok: %p\n", ptr);  /* ok to print, should not be reached */
	return 0;
}

int test_3(duk_context *ctx) {
	void *ptr;
	size_t sz;

	duk_set_top(ctx, 0);
	sz = (size_t) 0xdeadbeef;
	ptr = duk_require_buffer(ctx, 0, &sz);
	printf("buffer ok: %p\n", ptr);  /* ok to print, should not be reached */
	return 0;
}

int test_4(duk_context *ctx) {
	void *ptr;
	size_t sz;

	duk_set_top(ctx, 0);
	sz = (size_t) 0xdeadbeef;
	ptr = duk_require_buffer(ctx, DUK_INVALID_INDEX, &sz);
	printf("buffer ok: %p\n", ptr);  /* ok to print, should not be reached */
	return 0;
}

void test(duk_context *ctx) {
	int rc;

	rc = duk_safe_call(ctx, test_1, 0, 1, DUK_INVALID_INDEX);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_2, 0, 1, DUK_INVALID_INDEX);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_3, 0, 1, DUK_INVALID_INDEX);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_4, 0, 1, DUK_INVALID_INDEX);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);
}

