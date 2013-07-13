/*===
0: 123
1: 345
rc=0 -> undefined
remove at 2 ok
remove at -1 ok
rc=1 -> Error: index out of bounds
remove at 0 ok
remove at -2 ok
rc=1 -> Error: index out of bounds
rc=1 -> Error: index out of bounds
===*/

void prep(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_push_int(ctx, 345);       /* -> [ 123 234 345 ] */
}

int test_1(duk_context *ctx) {
	int i, n;

	prep(ctx);
	duk_remove(ctx, -2);          /* -> [ 123 345 ] */

	n = duk_get_top(ctx);
	for (i = 0; i < n; i++) {
		printf("%d: %s\n", i, duk_to_string(ctx, i));
	}
	return 0;
}

int test_2(duk_context *ctx) {
	prep(ctx);
	duk_remove(ctx, 2);   /* -> [ 123 234 ]  (legal) */
	printf("remove at 2 ok\n");
	duk_remove(ctx, -1);  /* -> [ 123 ]  (legal) */
	printf("remove at -1 ok\n");
	duk_remove(ctx, 1);   /* (illegal) */
	printf("remove at 1 ok\n");
	return 0;
}

int test_3(duk_context *ctx) {
	prep(ctx);
	duk_remove(ctx, 0);   /* -> [ 234 345 ]  (legal) */
	printf("remove at 0 ok\n");
	duk_remove(ctx, -2);  /* -> [ 345 ]  (legal) */
	printf("remove at -2 ok\n");
	duk_remove(ctx, -2);   /* (illegal) */
	printf("remove at -2 ok\n");
	return 0;
}

int test_4(duk_context *ctx) {
	prep(ctx);
	duk_remove(ctx, DUK_INVALID_INDEX);  /* (illegal) */
	printf("remove at DUK_INVALID_INDEX ok\n");
	return 0;
}

void test(duk_context *ctx) {
	int rc;

	rc = duk_safe_call(ctx, test_1, 0, 1, DUK_INVALID_INDEX);
	printf("rc=%d -> %s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_2, 0, 1, DUK_INVALID_INDEX);
	printf("rc=%d -> %s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_3, 0, 1, DUK_INVALID_INDEX);
	printf("rc=%d -> %s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_4, 0, 1, DUK_INVALID_INDEX);
	printf("rc=%d -> %s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);
}

