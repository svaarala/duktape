/*===
*** test_encode (duk_safe_call)
hex encode: 666f6f
top after: 2
==> rc=0, result='undefined'
*** test_decode (duk_safe_call)
hex decode: test string
top after: 2
==> rc=0, result='undefined'
*** test_decode_odd_length (duk_safe_call)
==> rc=1, result='TypeError: decode failed'
*** test_decode_invalid_char (duk_safe_call)
==> rc=1, result='TypeError: decode failed'
===*/

static duk_ret_t test_encode(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_string(ctx, "foo");
	duk_push_int(ctx, 123);  /* dummy */
	printf("hex encode: %s\n", duk_hex_encode(ctx, -2));
	printf("top after: %ld\n", (long) duk_get_top(ctx));  /* value + dummy */
	return 0;
}

static duk_ret_t test_decode(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_string(ctx, "7465737420737472696e67");
	duk_push_int(ctx, 321);  /* dummy */
	duk_hex_decode(ctx, -2);  /* buffer */
	printf("hex decode: %s\n", duk_to_string(ctx, -2));
	printf("top after: %ld\n", (long) duk_get_top(ctx));  /* value + dummy */
	duk_set_top(ctx, 0);
	return 0;
}

static duk_ret_t test_decode_odd_length(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_string(ctx, "7465737420737472696e6");  /* odd length */
	duk_push_int(ctx, 321);  /* dummy */
	duk_hex_decode(ctx, -2);  /* buffer */
	printf("hex decode: %s\n", duk_to_string(ctx, -2));
	printf("top after: %ld\n", (long) duk_get_top(ctx));  /* value + dummy */
	duk_set_top(ctx, 0);
	return 0;
}

static duk_ret_t test_decode_invalid_char(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_string(ctx, "7465737420737g72696e67");  /* invalid char */
	duk_push_int(ctx, 321);  /* dummy */
	duk_hex_decode(ctx, -2);  /* buffer */
	printf("hex decode: %s\n", duk_to_string(ctx, -2));
	printf("top after: %ld\n", (long) duk_get_top(ctx));  /* value + dummy */
	duk_set_top(ctx, 0);
	return 0;
}
void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_encode);
	TEST_SAFE_CALL(test_decode);
	TEST_SAFE_CALL(test_decode_odd_length);
	TEST_SAFE_CALL(test_decode_invalid_char);
}
