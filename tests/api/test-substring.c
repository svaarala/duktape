/*===
*** test_1 (duk_safe_call)
blen=5, clen=3, str="o\xe1\x88\xb4a"
blen=6, clen=4, str="o\xe1\x88\xb4ar"
blen=0, clen=0, str=""
blen=0, clen=0, str=""
final top: 1
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
==> rc=1, result='TypeError: string required, found 123456 (stack index 0)'
*** test_3 (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2'
*** test_4 (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
===*/

static void dump_string(duk_context *ctx) {
	const char *buf;
	duk_size_t i, len;

	buf = duk_get_lstring(ctx, -1, &len);
	printf("blen=%lu, clen=%lu, str=\"", (unsigned long) len, (unsigned long) duk_get_length(ctx, -1));
	for (i = 0; i < len; i++) {
		char c = buf[i];
		if (c >= 0x20 && c <= 0x7e) {
			printf("%c", c);
		} else {
			printf("\\x%02x", ((int) c) & 0xff);
		}
	}
	printf("\"\n");

	duk_pop(ctx);
}

static duk_ret_t test_1(duk_context *ctx) {
	/*
	 *  Test with a string containing non-ASCII to ensure indices are
	 *  treated correctly as char indices.
	 *
	 *  >>> u'foo\u1234ar'.encode('utf-8').encode('hex')
	 *  '666f6fe188b46172'
	 */
	const char *teststr = "666f6fe188b46172";

	duk_set_top(ctx, 0);

	duk_push_string(ctx, (const char *) teststr);
	duk_hex_decode(ctx, -1);
	duk_to_string(ctx, -1);

	/* basic case */
	duk_dup_top(ctx);
	duk_push_int(ctx, 123);  /* dummy */
	duk_substring(ctx, -2, 2, 5);  /* test index other than stack top */
	duk_pop(ctx);
	dump_string(ctx);

	/* end is clamped */
	duk_dup_top(ctx);
	duk_substring(ctx, -1, 2, 8);
	dump_string(ctx);

	/* start and end are clamped */
	duk_dup_top(ctx);
	duk_substring(ctx, -1, 10, 20);
	dump_string(ctx);

	/* start > end */
	duk_dup_top(ctx);
	duk_substring(ctx, -1, 4, 2);
	dump_string(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* non-string -> error */
static duk_ret_t test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123456);
	duk_substring(ctx, -1, 2, 4);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* invalid index */
static duk_ret_t test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_string(ctx, "foobar");
	duk_substring(ctx, -2, 2, 4);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* invalid index */
static duk_ret_t test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_string(ctx, "foobar");
	duk_substring(ctx, DUK_INVALID_INDEX, 2, 4);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
}
