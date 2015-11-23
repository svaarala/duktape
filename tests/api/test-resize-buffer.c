/*===
*** test_1 (duk_safe_call)
alloc to 16
16 bytes: 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2
resize to 64
64 bytes: 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3
resize to 7
7 bytes: 1 0 0 0 0 0 0
resize to 0
0 bytes:
final top: 1
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
alloc (fixed) to 16
16 bytes: 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2
resize (fixed) to 64
==> rc=1, result='TypeError: wrong buffer type'
*** test_3 (duk_safe_call)
non-buffer
resize (non-buffer) to 64
==> rc=1, result='TypeError: buffer required, found 'foo' (stack index -1)'
*** test_4 (duk_safe_call)
non-buffer
resize (invalid index) to 64
==> rc=1, result='TypeError: buffer required, found none (stack index 3)'
*** test_5 (duk_safe_call)
non-buffer
resize (DUK_INVALID_INDEX) to 64
==> rc=1, result='TypeError: buffer required, found none (stack index -2147483648)'
===*/

static void dump_buffer(duk_context *ctx) {
	unsigned char *p;
	duk_size_t i, sz;

	p = (unsigned char *) duk_require_buffer(ctx, -1, &sz);
	printf("%ld bytes:", (long) sz);
	for (i = 0; i < sz; i++) {
		printf(" %d", (int) p[i]);
	}
	printf("\n");
}

static duk_ret_t test_1(duk_context *ctx) {
	unsigned char *p;

	duk_set_top(ctx, 0);

	printf("alloc to 16\n");
	p = (unsigned char *) duk_push_dynamic_buffer(ctx, 16);
	p[0] = 1;
	p[15] = 2;
	dump_buffer(ctx);

	printf("resize to 64\n");
	p = (unsigned char *) duk_resize_buffer(ctx, -1, 64);
	p[63] = 3;
	dump_buffer(ctx);

	printf("resize to 7\n");
	p = (unsigned char *) duk_resize_buffer(ctx, -1, 7);
	dump_buffer(ctx);

	printf("resize to 0\n");
	p = (unsigned char *) duk_resize_buffer(ctx, -1, 0);
	dump_buffer(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* fixed buffer */
static duk_ret_t test_2(duk_context *ctx) {
	unsigned char *p;

	duk_set_top(ctx, 0);

	printf("alloc (fixed) to 16\n");
	p = (unsigned char *) duk_push_fixed_buffer(ctx, 16);
	p[0] = 1;
	p[15] = 2;
	dump_buffer(ctx);

	printf("resize (fixed) to 64\n");
	p = (unsigned char *) duk_resize_buffer(ctx, -1, 64);
	p[63] = 3;
	dump_buffer(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* non-buffer */
static duk_ret_t test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);

	printf("non-buffer\n");
	duk_push_string(ctx, "foo");

	printf("resize (non-buffer) to 64\n");
	(void) duk_resize_buffer(ctx, -1, 64);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* invalid index */
static duk_ret_t test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);

	printf("non-buffer\n");
	duk_push_string(ctx, "foo");

	printf("resize (invalid index) to 64\n");
	(void) duk_resize_buffer(ctx, 3, 64);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* DUK_INVALID_INDEX */
static duk_ret_t test_5(duk_context *ctx) {
	duk_set_top(ctx, 0);

	printf("non-buffer\n");
	duk_push_string(ctx, "foo");

	printf("resize (DUK_INVALID_INDEX) to 64\n");
	(void) duk_resize_buffer(ctx, DUK_INVALID_INDEX, 64);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
	TEST_SAFE_CALL(test_5);
}
