/*===
base64 encode: Zm9v
top after: 2
base64 decode: test string
top after: 2
===*/

static void test_encode(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_string(ctx, "foo");
	duk_push_int(ctx, 123);  /* dummy */
	printf("base64 encode: %s\n", duk_base64_encode(ctx, -2));
	printf("top after: %ld\n", (long) duk_get_top(ctx));  /* value + dummy */
}

static void test_decode(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_string(ctx, "dGVzdCBzdHJpbmc=");
	duk_push_int(ctx, 321);  /* dummy */
	duk_base64_decode(ctx, -2);  /* buffer */
	printf("base64 decode: %s\n", duk_to_string(ctx, -2));
	printf("top after: %ld\n", (long) duk_get_top(ctx));  /* value + dummy */
	duk_set_top(ctx, 0);
}

void test(duk_context *ctx) {
	test_encode(ctx);
	test_decode(ctx);
	/* FIXME: test decode error */
}
