/*===
*** test_1
[ 123 234 345 456 567 ]
[ 123 234 345 456 567 ]
[ 123 456 345 234 567 ]
[ 123 456 567 234 345 ]
[ 123 456 567 234 345 ]
final top: 5
rc=0, result=undefined
*** test_2a
rc=1, result=Error: index out of bounds
*** test_2b
rc=1, result=Error: index out of bounds
*** test_2c
rc=1, result=Error: index out of bounds
*** test_2d
rc=1, result=Error: index out of bounds
*** test_3a
rc=1, result=Error: index out of bounds
*** test_3b
rc=1, result=Error: index out of bounds
*** test_3c
rc=1, result=Error: index out of bounds
===*/

void dump_stack(duk_context *ctx) {
	int i, n;

	printf("[");
	n = duk_get_top(ctx);
	for (i = 0; i < n; i++) {
		printf(" %d", duk_get_int(ctx, i));
	}
	printf(" ]\n");
}

int test_1(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_push_int(ctx, 345);
	duk_push_int(ctx, 456);
	duk_push_int(ctx, 567);  /* [ 123 234 345 456 567 ] */
	dump_stack(ctx);

	/* no-op swap */
	duk_swap(ctx, -1, -1);
	dump_stack(ctx);

	/* actual swap */
	duk_swap(ctx, 1, -2);  /* -> [ 123 456 345 234 567 ] */
	dump_stack(ctx);

	/* swap top */
	duk_swap_top(ctx, -3); /* -> [ 123 456 567 234 345 ] */
	dump_stack(ctx);

	/* no-op swap top */
	duk_swap_top(ctx, 4);
	dump_stack(ctx);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

int test_2a(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_swap(ctx, -1, -3);  /* second index out of bounds */

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

int test_2b(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_swap(ctx, 6, 1);  /* first index out of bounds */

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

int test_2c(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_swap(ctx, DUK_INVALID_INDEX, 0);  /* first index DUK_INVALID_INDEX */

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

int test_2d(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_swap(ctx, 0, DUK_INVALID_INDEX);  /* second index DUK_INVALID_INDEX */

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

int test_3a(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_swap_top(ctx, 0);  /* empty stack */

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

int test_3b(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_swap_top(ctx, 2);  /* index out of bounds */

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

int test_3c(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_swap_top(ctx, DUK_INVALID_INDEX);  /* index is DUK_INVALID_INDEX */

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
	TEST(test_2a);
	TEST(test_2b);
	TEST(test_2c);
	TEST(test_2d);
	TEST(test_3a);
	TEST(test_3b);
	TEST(test_3c);
}

