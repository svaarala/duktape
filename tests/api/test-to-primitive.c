/*===
*** test_1 (duk_safe_call)
top: 19
index 0, ToString(result): 'undefined', type: 1 -> 1
index 1, ToString(result): 'null', type: 2 -> 2
index 2, ToString(result): 'true', type: 3 -> 3
index 3, ToString(result): 'false', type: 3 -> 3
index 4, ToString(result): '0', type: 4 -> 4
index 5, ToString(result): '1', type: 4 -> 4
index 6, ToString(result): 'NaN', type: 4 -> 4
index 7, ToString(result): 'Infinity', type: 4 -> 4
index 8, ToString(result): '', type: 5 -> 5
index 9, ToString(result): 'foo', type: 5 -> 5
index 10, ToString(result): '[object Object]', type: 6 -> 5
index 11, ToString(result): '123.456', type: 6 -> 4
index 12, ToString(result): '[object Thread]', type: 6 -> 5
index 13, ToString(result): '', type: 7 -> 7
index 14, ToString(result): '', type: 7 -> 7
index 15, ToString(result): '', type: 7 -> 7
index 16, ToString(result): '', type: 7 -> 7
index 17, ToString(result): 'null', type: 8 -> 8
index 18, ToString(result): '0xdeadbeef', type: 8 -> 8
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
==> rc=1, result='Error: invalid stack index 3'
*** test_3 (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
===*/

/* XXX: coverage is pretty poor, e.g. different hints are not tested.
 * They are indirectly covered by Ecmascript tests to some extent, though.
 */

static duk_ret_t test_1(duk_context *ctx) {
	duk_idx_t i, n;

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
	duk_push_number(ctx, 123.456);
	duk_to_object(ctx, -1);  /* Number(123.456) */
	duk_push_thread(ctx);
	duk_push_fixed_buffer(ctx, 0);
	duk_push_fixed_buffer(ctx, 1024);
	duk_push_dynamic_buffer(ctx, 0);
	duk_push_dynamic_buffer(ctx, 1024);
	duk_push_pointer(ctx, (void *) NULL);
	duk_push_pointer(ctx, (void *) 0xdeadbeef);

	n = duk_get_top(ctx);
	printf("top: %ld\n", (long) n);
	for (i = 0; i < n; i++) {
		duk_int_t t1, t2;

		t1 = duk_get_type(ctx, i);
		duk_to_primitive(ctx, i, DUK_HINT_NONE);
		t2 = duk_get_type(ctx, i);

		printf("index %ld, ToString(result): '%s', type: %ld -> %ld\n",
		       (long) i, duk_to_string(ctx, i), (long) t1, (long) t2);
	}

	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_primitive(ctx, 3, DUK_HINT_NONE);
	printf("index 3 OK\n");
	return 0;
}

static duk_ret_t test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_primitive(ctx, DUK_INVALID_INDEX, DUK_HINT_NONE);
	printf("index DUK_INVALID_INDEX OK\n");
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
}
