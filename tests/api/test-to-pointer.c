/*===
*** test_1 (duk_safe_call)
top: 18
index 0, ptr-is-NULL: 1, type: 1 -> 8
index 1, ptr-is-NULL: 1, type: 2 -> 8
index 2, ptr-is-NULL: 1, type: 3 -> 8
index 3, ptr-is-NULL: 1, type: 3 -> 8
index 4, ptr-is-NULL: 1, type: 4 -> 8
index 5, ptr-is-NULL: 1, type: 4 -> 8
index 6, ptr-is-NULL: 1, type: 4 -> 8
index 7, ptr-is-NULL: 1, type: 4 -> 8
index 8, ptr-is-NULL: 0, type: 5 -> 8
index 9, ptr-is-NULL: 0, type: 5 -> 8
index 10, ptr-is-NULL: 0, type: 6 -> 8
index 11, ptr-is-NULL: 0, type: 6 -> 8
index 12, ptr-is-NULL: 0, type: 7 -> 8
index 13, ptr-is-NULL: 0, type: 7 -> 8
index 14, ptr-is-NULL: 0, type: 7 -> 8
index 15, ptr-is-NULL: 0, type: 7 -> 8
index 16, ptr-is-NULL: 1, type: 8 -> 8
pointer: (nil)
index 17, ptr-is-NULL: 0, type: 8 -> 8
pointer: 0xdeadbeef
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
==> rc=1, result='Error: invalid stack index 3'
*** test_3 (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
===*/

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
		void *ptr;
		duk_int_t t1, t2;

		t1 = duk_get_type(ctx, i);
		ptr = duk_to_pointer(ctx, i);
		t2 = duk_get_type(ctx, i);

		printf("index %ld, ptr-is-NULL: %d, type: %ld -> %ld\n",
		       (long) i, (ptr == NULL ? 1 : 0), (long) t1, (long) t2);
		if (t1 == DUK_TYPE_POINTER) {
			/* check that pointer is retained as is (can safely print) */
			printf("pointer: %p\n", ptr);
		}
	}

	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_pointer(ctx, 3);
	printf("index 3 OK\n");
	return 0;
}

static duk_ret_t test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_to_pointer(ctx, DUK_INVALID_INDEX);
	printf("index DUK_INVALID_INDEX OK\n");
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
}
