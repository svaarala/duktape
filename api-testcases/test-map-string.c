/*===
*** test_1 (duk_safe_call)
test 1
result: 'TEST_STRING'
test 2
result: 'FOOXBAR'
final top: 0
==> rc=0, result='undefined'
===*/

static duk_codepoint_t map_char(void *udata, duk_codepoint_t codepoint) {
	if (codepoint >= (duk_codepoint_t) 'a' && codepoint <= (duk_codepoint_t) 'z') {
		/* Convert ASCII to uppercase. */
		return codepoint - (duk_codepoint_t) 'a' + (duk_codepoint_t) 'A';
	} else if (codepoint == 0x1234) {
		/* Convert U+1234 to 'X' */
		return (duk_codepoint_t) 'X';
	}
	return codepoint;
}

static duk_ret_t test_1(duk_context *ctx) {
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

	/* XXX: error cases */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
