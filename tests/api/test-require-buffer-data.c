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
buffer: ptr-is-NULL=0, sz=12
buffer
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
==> rc=1, result='TypeError: buffer required, found null (stack index 0)'
*** test_3 (duk_safe_call)
==> rc=1, result='TypeError: buffer required, found none (stack index 0)'
*** test_4 (duk_safe_call)
==> rc=1, result='TypeError: buffer required, found none (stack index -2147483648)'
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
	duk_eval_string(ctx, "(function () { return new Uint32Array(16).subarray(3, 6); })()");

	for (i = 0; i < 5; i++) {
		sz = (duk_size_t) 0xdeadbeefUL;
		ptr = duk_require_buffer_data(ctx, i, &sz);
		printf("buffer: ptr-is-NULL=%d, sz=%ld\n",
		       (sz == 0 ? -1 : (ptr == NULL ? 1 : 0)), (long) sz);

		/* NULL pointer */
		sz = (duk_size_t) 0xdeadbeefUL;
		ptr = duk_require_buffer_data(ctx, i, NULL);
		printf("buffer\n");
	}

	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	void *ptr;
	duk_size_t sz;

	duk_set_top(ctx, 0);
	duk_push_null(ctx);
	sz = (duk_size_t) 0xdeadbeefUL;
	ptr = duk_require_buffer_data(ctx, 0, &sz);
	printf("buffer ok: %p\n", ptr);  /* ok to print, should not be reached */
	return 0;
}

static duk_ret_t test_3(duk_context *ctx) {
	void *ptr;
	duk_size_t sz;

	duk_set_top(ctx, 0);
	sz = (duk_size_t) 0xdeadbeefUL;
	ptr = duk_require_buffer_data(ctx, 0, &sz);
	printf("buffer ok: %p\n", ptr);  /* ok to print, should not be reached */
	return 0;
}

static duk_ret_t test_4(duk_context *ctx) {
	void *ptr;
	duk_size_t sz;

	duk_set_top(ctx, 0);
	sz = (duk_size_t) 0xdeadbeefUL;
	ptr = duk_require_buffer_data(ctx, DUK_INVALID_INDEX, &sz);
	printf("buffer ok: %p\n", ptr);  /* ok to print, should not be reached */
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
}
