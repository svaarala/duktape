/*===
0: 123
1: 345
===*/

void test(duk_context *ctx) {
	int i, n;

	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_push_int(ctx, 345);       /* -> [ 123 234 345 ] */
	duk_remove(ctx, -2);          /* -> [ 123 345 ] */

	n = duk_get_top(ctx);
	for (i = 0; i < n; i++) {
		printf("%d: %s\n", i, duk_to_string(ctx, i));
	}
}

