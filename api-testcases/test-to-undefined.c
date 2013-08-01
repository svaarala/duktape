/*===
top: 18
index 0, is-undefined: 1
index 1, is-undefined: 1
index 2, is-undefined: 1
index 3, is-undefined: 1
index 4, is-undefined: 1
index 5, is-undefined: 1
index 6, is-undefined: 1
index 7, is-undefined: 1
index 8, is-undefined: 1
index 9, is-undefined: 1
index 10, is-undefined: 1
index 11, is-undefined: 1
index 12, is-undefined: 1
index 13, is-undefined: 1
index 14, is-undefined: 1
index 15, is-undefined: 1
index 16, is-undefined: 1
index 17, is-undefined: 1
rc=0, result=undefined
rc=1, result=Error: index out of bounds
rc=1, result=Error: index out of bounds
===*/

int test_1(duk_context *ctx) {
	int i, n;

	duk_set_top(ctx, 0);
	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_true(ctx);
	duk_push_false(ctx);
	duk_push_int(ctx, 0);
	duk_push_int(ctx, 1);
	duk_push_nan(ctx);
	duk_push_number(ctx, INFINITY);
	duk_push_string(ctx, "");
	duk_push_string(ctx, "foo");
	duk_push_object(ctx);
	duk_push_thread(ctx);
	duk_push_fixed_buffer(ctx, 0);
	duk_push_fixed_buffer(ctx, 1024);
	duk_push_dynamic_buffer(ctx, 0);
	duk_push_dynamic_buffer(ctx, 1024);
	duk_push_pointer(ctx, (void *) NULL);
	duk_push_pointer(ctx, (void *) 0xdeadbeef);

	n = duk_get_top(ctx);
	printf("top: %d\n", n);
	for (i = 0; i < n; i++) {
		duk_to_undefined(ctx, i);
		printf("index %d, is-undefined: %d\n", i, duk_is_undefined(ctx, i));
	}

	return 0;
}

int test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_undefined(ctx, 3);
	printf("index 3 OK\n");
	return 0;
}

int test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_undefined(ctx, DUK_INVALID_INDEX);
	printf("index DUK_INVALID_INDEX OK\n");
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
}

