/*===
hex encode: 666f6f
top after: 2
hex decode: test string
top after: 2
===*/

void test_encode(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_string(ctx, "foo");
	duk_push_int(ctx, 123);  /* dummy */
	printf("hex encode: %s\n", duk_hex_encode(ctx, -2));
	printf("top after: %ld\n", (long) duk_get_top(ctx));  /* value + dummy */
}

void test_decode(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_string(ctx, "7465737420737472696e67");
	duk_push_int(ctx, 321);  /* dummy */
	duk_hex_decode(ctx, -2);  /* buffer */
	printf("hex decode: %s\n", duk_to_string(ctx, -2));
	printf("top after: %ld\n", (long) duk_get_top(ctx));  /* value + dummy */
	duk_set_top(ctx, 0);
}

void test(duk_context *ctx) {
	test_encode(ctx);
	test_decode(ctx);
	/* FIXME: test decode error */
}

