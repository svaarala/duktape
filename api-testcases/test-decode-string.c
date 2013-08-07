/*===
test 1
codepoint: 116
codepoint: 101
codepoint: 115
codepoint: 116
codepoint: 95
codepoint: 115
codepoint: 116
codepoint: 114
codepoint: 105
codepoint: 110
codepoint: 103
test 2
codepoint: 102
codepoint: 111
codepoint: 111
codepoint: 4660
codepoint: 98
codepoint: 97
codepoint: 114
final top: 0
===*/

static void decode_char(void *udata, int codepoint) {
	printf("codepoint: %d\n", codepoint);
}

void test(duk_context *ctx) {
	printf("test 1\n");
	duk_push_string(ctx, "test_string");
	duk_decode_string(ctx, -1, decode_char, NULL);
	duk_pop(ctx);

	printf("test 2\n");
	duk_push_string(ctx, "foo" "\xe1\x88\xb4" "bar");
	duk_decode_string(ctx, -1, decode_char, NULL);
	duk_pop(ctx);

	/* FIXME: error cases */

	printf("final top: %d\n", duk_get_top(ctx));
}
