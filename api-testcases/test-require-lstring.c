/*===
*** test_1 (duk_safe_call)
string: foo (7)
string: foo
string:  (0)
string: 
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
==> rc=1, result='TypeError: not string'
*** test_3 (duk_safe_call)
==> rc=1, result='TypeError: not string'
*** test_4 (duk_safe_call)
==> rc=1, result='TypeError: not string'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	const char *p;
	duk_size_t sz;

	duk_set_top(ctx, 0);
	duk_push_lstring(ctx, "foo\0bar", 7);
	duk_push_string(ctx, "");

	sz = (duk_size_t) 0xdeadbeef;
	p = duk_require_lstring(ctx, 0, &sz);
	printf("string: %s (%ld)\n", p, (long) sz);

	sz = (duk_size_t) 0xdeadbeef;
	p = duk_require_lstring(ctx, 0, NULL);
	printf("string: %s\n", p);

	sz = (duk_size_t) 0xdeadbeef;
	p = duk_require_lstring(ctx, 1, &sz);
	printf("string: %s (%ld)\n", p, (long) sz);

	sz = (duk_size_t) 0xdeadbeef;
	p = duk_require_lstring(ctx, 1, NULL);
	printf("string: %s\n", p);
	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	const char *p;
	duk_size_t sz;

	duk_set_top(ctx, 0);
	duk_push_null(ctx);

	p = duk_require_lstring(ctx, 0, &sz);
	printf("string: %s (%ld)\n", p, (long) sz);
	return 0;
}

static duk_ret_t test_3(duk_context *ctx) {
	const char *p;
	duk_size_t sz;

	duk_set_top(ctx, 0);

	p = duk_require_lstring(ctx, 0, &sz);
	printf("string: %s (%ld)\n", p, (long) sz);
	return 0;
}

static duk_ret_t test_4(duk_context *ctx) {
	const char *p;
	duk_size_t sz;

	duk_set_top(ctx, 0);

	p = duk_require_lstring(ctx, DUK_INVALID_INDEX, &sz);
	printf("string: %s (%ld)\n", p, (long) sz);
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
}
