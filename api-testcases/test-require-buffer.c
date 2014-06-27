/*===
*** test_1 (duk_safe_call)
buffer: ptr-is-NULL=0, sz=1024
buffer
buffer: ptr-is-NULL=-1, sz=0
buffer
buffer: ptr-is-NULL=0, sz=1024
buffer
buffer: ptr-is-NULL=-1, sz=0
buffer
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
==> rc=1, result='TypeError: not buffer'
*** test_3 (duk_safe_call)
==> rc=1, result='TypeError: not buffer'
*** test_4 (duk_safe_call)
==> rc=1, result='TypeError: not buffer'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	void *ptr;
	duk_size_t sz;
	int i;

	duk_set_top(ctx, 0);
	duk_push_fixed_buffer(ctx, 1024);
	duk_push_fixed_buffer(ctx, 0);
	duk_push_dynamic_buffer(ctx, 1024);
	duk_push_dynamic_buffer(ctx, 0);

	for (i = 0; i < 4; i++) {
		sz = (duk_size_t) 0xdeadbeef;
		ptr = duk_require_buffer(ctx, i, &sz);
		printf("buffer: ptr-is-NULL=%d, sz=%ld\n",
		       (sz == 0 ? -1 : (ptr == NULL ? 1 : 0)), (long) sz);

		sz = (duk_size_t) 0xdeadbeef;
		ptr = duk_require_buffer(ctx, i, NULL);
		printf("buffer\n");
	}

	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	void *ptr;
	duk_size_t sz;

	duk_set_top(ctx, 0);
	duk_push_null(ctx);
	sz = (duk_size_t) 0xdeadbeef;
	ptr = duk_require_buffer(ctx, 0, &sz);
	printf("buffer ok: %p\n", ptr);  /* ok to print, should not be reached */
	return 0;
}

static duk_ret_t test_3(duk_context *ctx) {
	void *ptr;
	duk_size_t sz;

	duk_set_top(ctx, 0);
	sz = (duk_size_t) 0xdeadbeef;
	ptr = duk_require_buffer(ctx, 0, &sz);
	printf("buffer ok: %p\n", ptr);  /* ok to print, should not be reached */
	return 0;
}

static duk_ret_t test_4(duk_context *ctx) {
	void *ptr;
	duk_size_t sz;

	duk_set_top(ctx, 0);
	sz = (duk_size_t) 0xdeadbeef;
	ptr = duk_require_buffer(ctx, DUK_INVALID_INDEX, &sz);
	printf("buffer ok: %p\n", ptr);  /* ok to print, should not be reached */
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
}
