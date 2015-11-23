/*===
*** test_1 (duk_safe_call)
top: 3
index 0, type 6 -> 5, result: [object Object]
index 1, type 6 -> 4, result: 123
index 2, type 6 -> 3, result: true
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
==> rc=1, result='TypeError: object required, found 123 (stack index 0)'
*** test_3 (duk_safe_call)
==> rc=1, result='Error: invalid stack index 3'
*** test_4 (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
===*/

/* XXX: this test is missing a lot of coverage, like different hints,
 * different kinds of objects.  Much of this behavior is tested by the
 * Ecmascript tests.
 */

static duk_ret_t test_1(duk_context *ctx) {
	duk_idx_t i, n;

	duk_set_top(ctx, 0);
	duk_push_object(ctx);
	duk_push_int(ctx, 123);
	duk_to_object(ctx, -1);   /* ToObject(123) is a Number object */
	duk_push_true(ctx);
	duk_to_object(ctx, -1);   /* ToObject(true) is a Boolean object */

	n = duk_get_top(ctx);
	printf("top: %ld\n", (long) n);
	for (i = 0; i < n; i++) {
		duk_int_t t1, t2;

		t1 = duk_get_type(ctx, i);
		duk_to_defaultvalue(ctx, i, DUK_HINT_NONE);
		t2 = duk_get_type(ctx, i);
		printf("index %ld, type %ld -> %ld, result: %s\n",
		       (long) i, (long) t1, (long) t2, duk_to_string(ctx, i));
	}

	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	/* non-object input */
	duk_set_top(ctx, 0);
	duk_push_int(ctx, 123);
	duk_to_defaultvalue(ctx, 0, DUK_HINT_NONE);
	printf("non-object value OK\n");
	return 0;

}

static duk_ret_t test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_defaultvalue(ctx, 3, DUK_HINT_NONE);
	printf("index 3 OK\n");
	return 0;
}

static duk_ret_t test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_defaultvalue(ctx, DUK_INVALID_INDEX, DUK_HINT_NONE);
	printf("index DUK_INVALID_INDEX OK\n");
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
}
