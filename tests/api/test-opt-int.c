/*===
*** test_basic (duk_safe_call)
top: 18
index 0: value 123, DUK_INT_MIN=0, DUK_INT_MAX=0
index 1: TypeError: number required, found null (stack index 1)
index 2: TypeError: number required, found true (stack index 2)
index 3: TypeError: number required, found false (stack index 3)
index 4: TypeError: number required, found 'foo' (stack index 4)
index 5: TypeError: number required, found '123' (stack index 5)
index 6: value -2147483648, DUK_INT_MIN=1, DUK_INT_MAX=0
index 7: value -2147483648, DUK_INT_MIN=1, DUK_INT_MAX=0
index 8: value -3, DUK_INT_MIN=0, DUK_INT_MAX=0
index 9: value 0, DUK_INT_MIN=0, DUK_INT_MAX=0
index 10: value 0, DUK_INT_MIN=0, DUK_INT_MAX=0
index 11: value 3, DUK_INT_MIN=0, DUK_INT_MAX=0
index 12: value 123456789, DUK_INT_MIN=0, DUK_INT_MAX=0
index 13: value 2147483647, DUK_INT_MIN=0, DUK_INT_MAX=1
index 14: value 2147483647, DUK_INT_MIN=0, DUK_INT_MAX=1
index 15: value 2147483647, DUK_INT_MIN=0, DUK_INT_MAX=1
index 16: value 0, DUK_INT_MIN=0, DUK_INT_MAX=0
index 17: TypeError: number required, found [object Object] (stack index 17)
index 18: value 123, DUK_INT_MIN=0, DUK_INT_MAX=0
==> rc=0, result='undefined'
===*/

static duk_ret_t safe_helper(duk_context *ctx, void *udata) {
	duk_idx_t idx = (duk_idx_t) udata & 0xffffffffUL;
	duk_int_t v = duk_opt_int(ctx, idx, 123);
	(void) udata;

	printf("index %ld: value %ld, DUK_INT_MIN=%d, DUK_INT_MAX=%d\n",
	       (long) idx, (long) v,
	       (v == DUK_INT_MIN),
	       (v == DUK_INT_MAX));
	return 0;
}

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	duk_idx_t i, n;
	duk_int_t rc;

	(void) udata;

	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_true(ctx);
	duk_push_false(ctx);
	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "123");
	duk_push_number(ctx, -INFINITY);
	duk_push_number(ctx, ((duk_double_t) DUK_INT_MIN) * 2.0);
	duk_push_number(ctx, -3.9);
	duk_push_number(ctx, -0.0);
	duk_push_number(ctx, +0.0);
	duk_push_number(ctx, +3.9);
	duk_push_number(ctx, +123456789.0);
	duk_push_number(ctx, ((duk_double_t) DUK_INT_MAX) * 2.0);
	duk_push_number(ctx, ((duk_double_t) DUK_UINT_MAX) * 2.0);
	duk_push_number(ctx, +INFINITY);
	duk_push_nan(ctx);
	duk_push_object(ctx);

	n = duk_get_top(ctx);
	printf("top: %ld\n", (long) n);
	for (i = 0; i <= n; i++) {
		rc = duk_safe_call(ctx, safe_helper, (void *) i, 0, 1);
		if (rc != DUK_EXEC_SUCCESS) {
			printf("index %ld: %s\n", (long) i, duk_safe_to_string(ctx, -1));
		}
		duk_pop(ctx);
	}

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
