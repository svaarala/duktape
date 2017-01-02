/*===
top: 18
index 0: value 0
index 1: value 0
index 2: value 0
index 3: value 0
index 4: value 0
index 5: value 0
index 6: value DUK_INT_MIN
index 7: value DUK_INT_MIN
index 8: value -3
index 9: value 0
index 10: value 0
index 11: value 3
index 12: value 123456789
index 13: value DUK_INT_MAX
index 14: value DUK_INT_MAX
index 15: value DUK_INT_MAX
index 16: value 0
index 17: value 0
index 18: value 0
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
		duk_int_t v;

		v = duk_get_int(ctx, i);

		printf("index %ld: value ", (long) i);
		if (v == DUK_INT_MIN) {
			printf("DUK_INT_MIN");
		} else if (v == DUK_INT_MAX) {
			printf("DUK_INT_MAX");
		} else {
			printf("%ld", (long) v);
		}
		printf("\n");
	}
}
