/*===
top: 11
index 0: boolean 0
index 1: boolean 0
index 2: boolean 1
index 3: boolean 0
index 4: boolean 0
index 5: boolean 0
index 6: boolean 0
index 7: boolean 0
index 8: boolean 0
index 9: boolean 0
index 10: boolean 0
===*/

void test(duk_context *ctx) {
	duk_idx_t i, n;

	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_true(ctx);
	duk_push_false(ctx);
	duk_push_string(ctx, "");
	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "true");
	duk_push_string(ctx, "false");
	duk_push_int(ctx, 0);
	duk_push_int(ctx, 123);
	duk_push_object(ctx);

	n = duk_get_top(ctx);
	printf("top: %ld\n", (long) n);
	for (i = 0; i < n; i++) {
		printf("index %ld: boolean %d\n", (long) i, (int) duk_get_boolean(ctx, i));
	}
}
