/*===
*** test_1 (duk_safe_call)
0: clen=10, trimmed='foo'
1: clen=17, trimmed='foo bar'
final top: 2
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
==> rc=1, result='TypeError: string required, found 123 (stack index 0)'
*** test_3 (duk_safe_call)
==> rc=1, result='Error: invalid stack index 4'
*** test_4 (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	duk_idx_t i, n;

	duk_set_top(ctx, 0);

	/* simple */
	duk_push_string(ctx, "   foo   \n");

	/*
	 *  >>> u'\u00a0\ufeff'.encode('utf-8').encode('hex')
	 *  'c2a0efbbbf'
	 *
	 *  >>> u'\u2028\u2029'.encode('utf-8').encode('hex')
	 *  'e280a8e280a9'
	 */

	duk_push_string(ctx, "\xc2\xa0\xef\xbb\xbf\x0a\xe2\x80\xa8\xe2\x80\xa9"
	                     "foo bar"
	                     "\xc2\xa0\xef\xbb\xbf\x0a\xe2\x80\xa8\xe2\x80\xa9");


	n = duk_get_top(ctx);
	for (i = 0; i < n; i++) {
		duk_size_t sz;

		sz = duk_get_length(ctx, i);
		duk_trim(ctx, i);
		printf("%ld: clen=%lu, trimmed='%s'\n",
		       (long) i, (unsigned long) sz, duk_get_string(ctx, i));
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123);
	duk_trim(ctx, -1);
	printf("trimmed non-string, should not happen\n");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123);
	duk_trim(ctx, 4);
	printf("trimmed invalid index, should not happen\n");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123);
	duk_trim(ctx, DUK_INVALID_INDEX);
	printf("trimmed DUK_INVALID_INDEX, should not happen\n");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
}
