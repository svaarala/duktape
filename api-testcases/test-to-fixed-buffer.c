/*===
*** test_1
16 bytes: 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2
16 bytes: 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2
16 bytes: 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2
final top: 1
rc=0, result=undefined
*** test_2
16 bytes: 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2
16 bytes: 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2
final top: 1
rc=0, result=undefined
*** test_3
rc=1, result=TypeError: incorrect type, expected tag 7
*** test_4
rc=1, result=Error: invalid index: 3
*** test_5
rc=1, result=Error: invalid index: -2147483648
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

/* dynamic buffer */
int test_1(duk_context *ctx) {
	unsigned char *p;

	duk_set_top(ctx, 0);

	p = (unsigned char *) duk_push_dynamic_buffer(ctx, 16);
	p[0] = 1;
	p[15] = 2;
	dump_buffer(ctx);

	duk_to_fixed_buffer(ctx, -1);
	dump_buffer(ctx);

	/* second time should be a no-op */
	duk_to_fixed_buffer(ctx, -1);
	dump_buffer(ctx);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* fixed buffer */
int test_2(duk_context *ctx) {
	unsigned char *p;

	duk_set_top(ctx, 0);

	p = (unsigned char *) duk_push_fixed_buffer(ctx, 16);
	p[0] = 1;
	p[15] = 2;
	dump_buffer(ctx);

	duk_to_fixed_buffer(ctx, -1);
	dump_buffer(ctx);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* non-buffer */
int test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_string(ctx, "foo");

	duk_to_fixed_buffer(ctx, -1);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* invalid index */
int test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_to_fixed_buffer(ctx, 3);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* DUK_INVALID_INDEX */
int test_5(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_to_fixed_buffer(ctx, DUK_INVALID_INDEX);

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
	TEST(test_5);  /* FIXME: exposes DUK_INVALID_INDEX number constant */
}

