/*===
--- top=0
top=0, idx=0, type 0 -> (null)
--- top=3
top=3, idx=0, type 5 -> foo
top=3, idx=1, type 5 -> bar
top=3, idx=2, type 5 -> quux
top=3, idx=3, type 0 -> (null)
--- top=5
top=5, idx=0, type 5 -> foo
top=5, idx=1, type 5 -> bar
top=5, idx=2, type 5 -> quux
top=5, idx=3, type 1 -> undefined
top=5, idx=4, type 1 -> undefined
top=5, idx=5, type 0 -> (null)
--- top=2
top=2, idx=0, type 5 -> foo
top=2, idx=1, type 5 -> bar
top=2, idx=2, type 0 -> (null)
--- top=0
top=0, idx=0, type 0 -> (null)
===*/

void print_stack(duk_context *ctx) {
	int i, n, t;

	n = duk_get_top(ctx);
	printf("--- top=%d\n", n);
	for (i = 0; i <= n; i++) {
		/* Go one over top intentionally */
		t = duk_get_type(ctx, i);  /* before coercion */
		if (i < n) {
			duk_to_string(ctx, i);
		}
		printf("top=%d, idx=%d, type %d -> %s\n", n, i, t, duk_get_string(ctx, i));
	}
}

void prep(duk_context *ctx) {
	duk_set_top(ctx, 0);
	print_stack(ctx);
	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "bar");
	duk_push_string(ctx, "quux");
	print_stack(ctx);
}

int test_1(duk_context *ctx) {
	printf("test_1\n");
	prep(ctx);

	/* extend with undefined */
	duk_set_top(ctx, 5);
	print_stack(ctx);

	/* chop, using a negative top value */
	duk_set_top(ctx, -3);   /* = duk_set_top(ctx, 2) */
	print_stack(ctx);

	/* to empty */
	duk_set_top(ctx, 0);
	print_stack(ctx);

	return 0;
}

int test_2(duk_context *ctx) {
	printf("test_2\n");
	prep(ctx);

	/* out of bounds negative top value */
	duk_set_top(ctx, -4);
	printf("duk_set_top for -4 ok\n");

	return 0;
}

int test_3(duk_context *ctx) {
	printf("test_3\n");
	prep(ctx);

	/* invalid top value */
	duk_set_top(ctx, DUK_INVALID_INDEX);
	printf("duk_set_top for DUK_INVALID_INDEX ok\n");

	return 0;
}

int test_4(duk_context *ctx) {
	printf("test_4\n");
	prep(ctx);

	/* positive top value is too high compared to "checked" stack size */
	duk_set_top(ctx, 500);
	printf("duk_set_top for 500 ok\n");

	return 0;
}

int test_5(duk_context *ctx) {
	printf("test_5\n");
	prep(ctx);

	/* by "checking" the stack, set to 500 works */
	duk_require_stack(ctx, 500);
	duk_set_top(ctx, 500);
	printf("duk_set_top for 500 ok\n");

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

	rc = duk_safe_call(ctx, test_5, 0, 1, DUK_INVALID_INDEX);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);
}

