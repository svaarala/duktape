/*===
top: 19
index 0: length 3
index 1: length 4
index 2: length 0
index 3: length 123
index 4: length 0
index 5: length 123
index 6: length 0
index 7: length 0
index 8: length 4294967295
index 9: length 0
index 10: length 0
index 11: length 0
index 12: length 0
index 13: length 1234
index 14: length 2345
index 15: length 0
index 16: length 0
index 17: length 0
index 18: length 0
===*/

void test(duk_context *ctx) {
	duk_idx_t i, n;

	/* 0 */
	duk_push_string(ctx, "foo");

	/* 1 */
	duk_push_string(ctx, "\xe1\x88\xb4xyz");  /* 4 chars, first char utf-8 encoded U+1234 */

	/* 2 */
	duk_push_object(ctx);  /* no length property */

	/* 3 */
	duk_push_object(ctx);  /* length: 123 */
	duk_push_int(ctx, 123);
	duk_put_prop_string(ctx, -2, "length");

	/* 4 */
	duk_push_object(ctx);  /* length: "bar" */
	duk_push_string(ctx, "bar");
	duk_put_prop_string(ctx, -2, "length");

	/* 5 */
	duk_push_object(ctx);  /* length: 123.9, fractional number */
	duk_push_number(ctx, 123.9);
	duk_put_prop_string(ctx, -2, "length");

	/* 6 */
	duk_push_object(ctx);  /* length: negative but within 32-bit range after ToInteger() */
	duk_push_number(ctx, -0.9);
	duk_put_prop_string(ctx, -2, "length");

	/* 7 */
	duk_push_object(ctx);  /* length: negative, outside 32-bit range */
	duk_push_number(ctx, -1);
	duk_put_prop_string(ctx, -2, "length");

	/* 8 */
	duk_push_object(ctx);  /* length: outside 32-bit range but within range after ToInteger() */
	duk_push_number(ctx, 4294967295.9);
	duk_put_prop_string(ctx, -2, "length");

	/* 9 */
	duk_push_object(ctx);  /* length: outside 32-bit range */
	duk_push_number(ctx, 4294967296);
	duk_put_prop_string(ctx, -2, "length");

	/* 10 */
	duk_push_object(ctx);  /* length: nan */
	duk_push_nan(ctx);
	duk_put_prop_string(ctx, -2, "length");

	/* 11 */
	duk_push_object(ctx);  /* length: +Infinity */
	duk_push_number(ctx, INFINITY);
	duk_put_prop_string(ctx, -2, "length");

	/* 12 */
	duk_push_object(ctx);  /* length: -Infinity */
	duk_push_number(ctx, -INFINITY);
	duk_put_prop_string(ctx, -2, "length");

	/* 13 */
	duk_push_fixed_buffer(ctx, 1234);

	/* 14 */
	duk_push_dynamic_buffer(ctx, 2345);

	/* 15 */
	duk_push_undefined(ctx);

	/* 16 */
	duk_push_null(ctx);

	/* 17 */
	duk_push_true(ctx);

	/* 18 */
	duk_push_false(ctx);

	n = duk_get_top(ctx);
	printf("top: %ld\n", (long) n);
	for (i = 0; i < n; i++) {
		printf("index %ld: length %lu\n", (long) i,
		       (unsigned long) duk_get_length(ctx, i));
	}
}
