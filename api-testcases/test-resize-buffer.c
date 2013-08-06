/*===
*** test_1
alloc to 16
16 bytes: 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2
resize to 64
64 bytes: 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3
resize to 7
7 bytes: 1 0 0 0 0 0 0
resize to 0
0 bytes:
final top: 1
rc=0, result=undefined
*** test_2
alloc (fixed) to 16
16 bytes: 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2
resize (fixed) to 64
rc=1, result=TypeError: buffer is not dynamic
*** test_3
non-buffer
resize (non-buffer) to 64
rc=1, result=TypeError: incorrect type, expected tag 7
*** test_4
non-buffer
resize (invalid index) to 64
rc=1, result=TypeError: incorrect type, expected tag 7
*** test_5
non-buffer
resize (DUK_INVALID_INDEX) to 64
rc=1, result=TypeError: incorrect type, expected tag 7
===*/

void dump_buffer(duk_context *ctx) {
	unsigned char *p;
	size_t i, s;

	p = (unsigned char *) duk_require_buffer(ctx, -1, &s);
	printf("%d bytes:", (int) s);
	for (i = 0; i < s; i++) {
		printf(" %d", (int) p[i]);
	}
	printf("\n");
}

int test_1(duk_context *ctx) {
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

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* fixed buffer */
int test_2(duk_context *ctx) {
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

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* non-buffer */
int test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);

	printf("non-buffer\n");
	duk_push_string(ctx, "foo");

	printf("resize (non-buffer) to 64\n");
	(void) duk_resize_buffer(ctx, -1, 64);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* invalid index */
int test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);

	printf("non-buffer\n");
	duk_push_string(ctx, "foo");

	printf("resize (invalid index) to 64\n");
	(void) duk_resize_buffer(ctx, 3, 64);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* DUK_INVALID_INDEX */
int test_5(duk_context *ctx) {
	duk_set_top(ctx, 0);

	printf("non-buffer\n");
	duk_push_string(ctx, "foo");

	printf("resize (DUK_INVALID_INDEX) to 64\n");
	(void) duk_resize_buffer(ctx, DUK_INVALID_INDEX, 64);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

#define  TEST(func)  do { \
		printf("*** %s\n", #func); \
		rc = duk_safe_call(ctx, (func), 0, 1, DUK_INVALID_INDEX); \
		printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1)); \
		duk_pop(ctx); \
	} while (0)

void test(duk_context *ctx) {
	int rc;

	TEST(test_1);
	TEST(test_2);
	TEST(test_3);
	TEST(test_4);
	TEST(test_5);
}

