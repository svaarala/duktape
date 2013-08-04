/*===
*** test_1
0: clen=10, trimmed='foo'
1: clen=17, trimmed='foo bar'
final top: 2
rc=0, result=undefined
*** test_2
rc=1, result=TypeError: incorrect type, expected tag 5
*** test_3
rc=1, result=Error: invalid index: 4
*** test_4
rc=1, result=Error: invalid index: -2147483648
===*/

int test_1(duk_context *ctx) {
	int i, n;

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
		size_t sz;

		sz = duk_get_length(ctx, i);
		duk_trim(ctx, i);
		printf("%d: clen=%d, trimmed='%s'\n",
		       i, (int) sz, duk_get_string(ctx, i));
	}

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

int test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123);
	duk_trim(ctx, -1);
	printf("trimmed non-string, should not happen\n");

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

int test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123);
	duk_trim(ctx, 4);
	printf("trimmed invalid index, should not happen\n");

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

int test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_int(ctx, 123);
	duk_trim(ctx, DUK_INVALID_INDEX);
	printf("trimmed DUK_INVALID_INDEX, should not happen\n");

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

#define  TEST(func)  do { \
		printf("*** %s\n", #func); \
		rc = duk_safe_call(ctx, (func), 0, 1, DUK_INVALID_INDEX); \
		printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1)); \
		duk_pop(ctx); \
	} while (0)

void test(duk_context *ctx) {
	int rc;

	TEST(test_1);
	TEST(test_2);
	TEST(test_3);

	/* FIXME: this test prints DUK_INVALID_INDEX value in its error string now */
	TEST(test_4);
}
