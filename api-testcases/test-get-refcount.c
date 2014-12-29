/*
 *  duk_get_refcount()
 */

/*===
*** test_1 (duk_safe_call)
-6: valid_index=0 refcount 0
-5: valid_index=1 refcount 0
-4: valid_index=1 refcount 0
-3: valid_index=1 refcount 0
-2: valid_index=1 refcount 1
-1: valid_index=1 refcount 1
0: valid_index=1 refcount 0
1: valid_index=1 refcount 0
2: valid_index=1 refcount 0
3: valid_index=1 refcount 1
4: valid_index=1 refcount 1
5: valid_index=0 refcount 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	duk_idx_t i, n;

	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_number(ctx, 123.0);
	duk_push_string(ctx, "dummy string");
	duk_push_object(ctx);

	/* Test both negative and positive indexes, and off by one to also
	 * test invalid indices too.
	 */
	n = duk_get_top(ctx) + 1;
	for (i = -n; i < n; i++) {
		printf("%ld: valid_index=%d refcount %ld\n", (long) i, (int) duk_is_valid_index(ctx, i), (long) duk_get_refcount(ctx, i));
	}

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
