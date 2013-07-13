/*===
0: 123
1: foo
2: 234
3: 345
===*/

void test(duk_context *ctx) {
	int i, n;

	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_push_int(ctx, 345);       /* -> [ 123 234 345 ] */
	duk_push_string(ctx, "foo");  /* -> [ 123 234 345 "foo" ] */
	duk_insert(ctx, -3);          /* -> [ 123 "foo" 234 345 ] */

	n = duk_get_top(ctx);
	for (i = 0; i < n; i++) {
		printf("%d: %s\n", i, duk_to_string(ctx, i));
	}
}

