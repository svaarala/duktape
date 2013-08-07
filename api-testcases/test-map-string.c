/*===
test 1
result: 'TEST_STRING'
test 2
result: 'FOOXBAR'
final top: 0
===*/

static int map_char(void *udata, int codepoint) {
	if (codepoint >= (int) 'a' && codepoint <= (int) 'z') {
		/* Convert ASCII to uppercase. */
		return codepoint - (int) 'a' + (int) 'A';
	} else if (codepoint == 0x1234) {
		/* Convert U+1234 to 'X' */
		return (int) 'X';
	}
	return codepoint;
}

void test(duk_context *ctx) {
	printf("test 1\n");
	duk_push_string(ctx, "test_string");
	duk_map_string(ctx, -1, map_char, NULL);
	printf("result: '%s'\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	printf("test 2\n");
	duk_push_string(ctx, "foo" "\xe1\x88\xb4" "bar");
	duk_map_string(ctx, -1, map_char, NULL);
	printf("result: '%s'\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	/* FIXME: error cases */

	printf("final top: %d\n", duk_get_top(ctx));
}
