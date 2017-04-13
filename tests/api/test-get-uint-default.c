/*===
top: 18
index 0: value 123
index 1: value 123
index 2: value 123
index 3: value 123
index 4: value 123
index 5: value 123
index 6: value 0
index 7: value 0
index 8: value 0
index 9: value 0
index 10: value 0
index 11: value 3
index 12: value 123456789
index 13: value 4294967294
index 14: value DUK_UINT_MAX
index 15: value DUK_UINT_MAX
index 16: value 0
index 17: value 123
index 18: value 123
===*/

void test(duk_context *ctx) {
	duk_idx_t i, n;

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
		duk_uint_t v;

		v = duk_get_uint_default(ctx, i, 123);

		printf("index %ld: value ", (long) i);
		if (v == DUK_UINT_MAX) {
			printf("DUK_UINT_MAX");
		} else {
			printf("%lu", (unsigned long) v);
		}
		printf("\n");
	}
}
