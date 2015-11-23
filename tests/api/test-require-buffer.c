/*===
*** test_basic (duk_safe_call)
buffer: ptr-is-NULL=0, sz=1024
buffer
buffer: ptr-is-NULL=-1, sz=0
buffer
buffer: ptr-is-NULL=0, sz=1024
buffer
buffer: ptr-is-NULL=-1, sz=0
buffer
==> rc=0, result='undefined'
*** test_invalid_type (duk_safe_call)
==> rc=1, result='TypeError: buffer required, found null (stack index 0)'
*** test_invalid_index1 (duk_safe_call)
==> rc=1, result='TypeError: buffer required, found none (stack index 0)'
*** test_invalid_index2 (duk_safe_call)
==> rc=1, result='TypeError: buffer required, found none (stack index -2147483648)'
*** test_buffer_object (duk_safe_call)
==> rc=1, result='TypeError: buffer required, found [object ArrayBuffer] (stack index -1)'
===*/

static duk_ret_t test_basic(duk_context *ctx) {
	void *ptr;
	duk_size_t sz;
	int i;

	duk_set_top(ctx, 0);
	duk_push_fixed_buffer(ctx, 1024);
	duk_push_fixed_buffer(ctx, 0);
	duk_push_dynamic_buffer(ctx, 1024);
	duk_push_dynamic_buffer(ctx, 0);

	for (i = 0; i < 4; i++) {
		sz = (duk_size_t) 0xdeadbeefUL;
		ptr = duk_require_buffer(ctx, i, &sz);
		printf("buffer: ptr-is-NULL=%d, sz=%ld\n",
		       (sz == 0 ? -1 : (ptr == NULL ? 1 : 0)), (long) sz);

		/* NULL pointer */
		sz = (duk_size_t) 0xdeadbeefUL;
		ptr = duk_require_buffer(ctx, i, NULL);
		printf("buffer\n");
	}

	return 0;
}

static duk_ret_t test_invalid_type(duk_context *ctx) {
	void *ptr;
	duk_size_t sz;

	duk_set_top(ctx, 0);
	duk_push_null(ctx);
	sz = (duk_size_t) 0xdeadbeefUL;
	ptr = duk_require_buffer(ctx, 0, &sz);
	printf("buffer ok: %p\n", ptr);  /* ok to print, should not be reached */
	return 0;
}

static duk_ret_t test_invalid_index1(duk_context *ctx) {
	void *ptr;
	duk_size_t sz;

	duk_set_top(ctx, 0);
	sz = (duk_size_t) 0xdeadbeefUL;
	ptr = duk_require_buffer(ctx, 0, &sz);
	printf("buffer ok: %p\n", ptr);  /* ok to print, should not be reached */
	return 0;
}

static duk_ret_t test_invalid_index2(duk_context *ctx) {
	void *ptr;
	duk_size_t sz;

	duk_set_top(ctx, 0);
	sz = (duk_size_t) 0xdeadbeefUL;
	ptr = duk_require_buffer(ctx, DUK_INVALID_INDEX, &sz);
	printf("buffer ok: %p\n", ptr);  /* ok to print, should not be reached */
	return 0;
}

static duk_ret_t test_buffer_object(duk_context *ctx) {
	void *ptr;
	duk_size_t sz;

	/* duk_require_buffer() doesn't accept a buffer object */

	duk_set_top(ctx, 0);
	duk_eval_string(ctx, "new ArrayBuffer(16)");
	sz = (duk_size_t) 0xdeadbeefUL;
	ptr = duk_require_buffer(ctx, -1, &sz);
	printf("buffer ok: %p\n", ptr);  /* ok to print, should not be reached */
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_invalid_type);
	TEST_SAFE_CALL(test_invalid_index1);
	TEST_SAFE_CALL(test_invalid_index2);
	TEST_SAFE_CALL(test_buffer_object);
}
