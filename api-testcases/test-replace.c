/*===
0: 123
1: foo
2: 345
rc=0 -> undefined
replace at 3 ok
rc=1 -> Error: index out of bounds
replace at -4 ok
rc=1 -> Error: index out of bounds
rc=1 -> Error: index out of bounds
===*/

void prep(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_push_int(ctx, 345);       /* -> [ 123 234 345 ] */
	duk_push_string(ctx, "foo");  /* -> [ 123 234 345 "foo" ] */
}

int test_1(duk_context *ctx) {
	int i, n;

	prep(ctx);
	duk_replace(ctx, -3);         /* -> [ 123 "foo" 345 ] */

	n = duk_get_top(ctx);
	for (i = 0; i < n; i++) {
		printf("%d: %s\n", i, duk_to_string(ctx, i));
	}
	return 0;
}

int test_2(duk_context *ctx) {
	prep(ctx);
	duk_replace(ctx, 3);          /* -> [ 123 234 "foo" ]  (legal) */
	printf("replace at 3 ok\n");
	duk_replace(ctx, 3);          /* (illegal: index too high) */
	printf("replace at 3 ok\n");
	return 0;
}

int test_3(duk_context *ctx) {
	prep(ctx);
	duk_replace(ctx, -4);         /* -> [ "foo" 234 345 ]  (legal) */
	printf("replace at -4 ok\n");
	duk_replace(ctx, -4);         /* (illegal: index too low) */
	printf("replace at -4 ok\n");
	return 0;
}

int test_4(duk_context *ctx) {
	prep(ctx);
	duk_replace(ctx, DUK_INVALID_INDEX);  /* (illegal: invalid index) */
	printf("replace at DUK_INVALID_INDEX ok\n");
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

