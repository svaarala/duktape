/*===
top: 10
index 0: string '(null)'
index 1: string '(null)'
index 2: string '(null)'
index 3: string '(null)'
index 4: string ''
index 5: string 'foo'
index 6: string 'foo'
index 7: string 'ሴxyz'
index 8: string '(null)'
index 9: string '(null)'
===*/

void test(duk_context *ctx) {
	duk_idx_t i, n;

	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_true(ctx);
	duk_push_false(ctx);
	duk_push_string(ctx, "");
	duk_push_string(ctx, "foo");
	duk_push_lstring(ctx, "foo\0bar", 7);
	duk_push_string(ctx, "\xe1\x88\xb4xyz");  /* 4 chars, first char utf-8 encoded U+1234 */
	duk_push_nan(ctx);
	duk_push_object(ctx);

	n = duk_get_top(ctx);
	printf("top: %ld\n", (long) n);
	for (i = 0; i < n; i++) {
		printf("index %ld: string '%s'\n", (long) i, duk_get_string(ctx, i));
	}
}
