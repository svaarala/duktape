/*
 *  RegExp with a character class whose endpoint is unescaped and above BMP.
 *  This can only be created from C code in Duktape 2.x.
 */

/*===
*** test_1 (duk_safe_call)
true
true
true
true
true
true
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	/* >>> u'[\u0000-\U0010ffff]'.encode('utf-8').encode('hex')
	 * '5b002df48fbfbf5d'
	 * >>> u'\u101234'.encode('utf-8').encode('hex')
	 * 'e180923334'
	 */

	duk_eval_string(ctx,
		"(function (src, test1) {\n"
		"    var re = new RegExp(src, 'i');\n"
		"    print(re.test(test1));\n"
		"    print(re.test('abcdefg'));\n"
		"    print(re.test('\\u0000'));\n"
		"    print(re.test('\\u0001'));\n"
		"    print(re.test('\\ufffe'));\n"
		"    print(re.test('\\uffff'));\n"
		"})");
	duk_push_lstring(ctx, "\x5b" "\x00" "\x2d" "\xf4" "\x8f" "\xbf" "\xbf" "\x5d", 8);
	duk_push_lstring(ctx, "\xe1" "\x80" "\x92""\x33" "\x34", 5);
	duk_call(ctx, 2);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
