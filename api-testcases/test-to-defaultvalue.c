/*===
top: 3
index 0, type 6 -> 5, result: [object Object]
index 1, type 6 -> 4, result: 123
index 2, type 6 -> 3, result: true
rc=0, result=undefined
rc=1, result=TypeError: not object
rc=1, result=Error: invalid index: 3
rc=1, result=Error: invalid index: -2147483648
===*/

/* FIXME: this test is missing a lot of coverage, like different hints,
 * different kinds of objects.  Much of this behavior is tested by the
 * Ecmascript tests.
 */

int test_1(duk_context *ctx) {
	int i, n;

	duk_set_top(ctx, 0);
	duk_push_object(ctx);
	duk_push_int(ctx, 123);
	duk_to_object(ctx, -1);   /* ToObject(123) is a Number object */
	duk_push_true(ctx);
	duk_to_object(ctx, -1);   /* ToObject(true) is a Boolean object */

	n = duk_get_top(ctx);
	printf("top: %d\n", n);
	for (i = 0; i < n; i++) {
		int t1, t2;

		t1 = duk_get_type(ctx, i);
		duk_to_defaultvalue(ctx, i, DUK_HINT_NONE);
		t2 = duk_get_type(ctx, i);
		printf("index %d, type %d -> %d, result: %s\n", i, t1, t2, duk_to_string(ctx, i));
	}

	return 0;
}

int test_2(duk_context *ctx) {
	/* non-object input */
	duk_set_top(ctx, 0);
	duk_push_int(ctx, 123);
	duk_to_defaultvalue(ctx, 0, DUK_HINT_NONE);
	printf("non-object value OK\n");
	return 0;

}

int test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_defaultvalue(ctx, 3, DUK_HINT_NONE);
	printf("index 3 OK\n");
	return 0;
}

int test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_defaultvalue(ctx, DUK_INVALID_INDEX, DUK_HINT_NONE);
	printf("index DUK_INVALID_INDEX OK\n");
	return 0;
}

void test(duk_context *ctx) {
	int rc;

	rc = duk_safe_call(ctx, test_1, 0, 1);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_2, 0, 1);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_3, 0, 1);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	/* FIXME: this test now results in an error string containing the
	 * exact index, which is platform dependent.
	 */
	rc = duk_safe_call(ctx, test_4, 0, 1);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);
}

