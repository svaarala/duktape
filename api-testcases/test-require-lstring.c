/*===
string: foo (7)
string: foo
string:  (0)
string: 
rc=0, result=undefined
rc=1, result=TypeError: not string
rc=1, result=TypeError: not string
rc=1, result=TypeError: not string
===*/

int test_1(duk_context *ctx) {
	const char *p;
	size_t sz;

	duk_set_top(ctx, 0);
	duk_push_lstring(ctx, "foo\0bar", 7);
	duk_push_string(ctx, "");

	sz = (size_t) 0xdeadbeef;
	p = duk_require_lstring(ctx, 0, &sz);
	printf("string: %s (%d)\n", p, (int) sz);

	sz = (size_t) 0xdeadbeef;
	p = duk_require_lstring(ctx, 0, NULL);
	printf("string: %s\n", p);

	sz = (size_t) 0xdeadbeef;
	p = duk_require_lstring(ctx, 1, &sz);
	printf("string: %s (%d)\n", p, (int) sz);

	sz = (size_t) 0xdeadbeef;
	p = duk_require_lstring(ctx, 1, NULL);
	printf("string: %s\n", p);
	return 0;
}

int test_2(duk_context *ctx) {
	const char *p;
	size_t sz;

	duk_set_top(ctx, 0);
	duk_push_null(ctx);

	p = duk_require_lstring(ctx, 0, &sz);
	printf("string: %s (%d)\n", p, (int) sz);
	return 0;
}

int test_3(duk_context *ctx) {
	const char *p;
	size_t sz;

	duk_set_top(ctx, 0);

	p = duk_require_lstring(ctx, 0, &sz);
	printf("string: %s (%d)\n", p, (int) sz);
	return 0;
}

int test_4(duk_context *ctx) {
	const char *p;
	size_t sz;

	duk_set_top(ctx, 0);

	p = duk_require_lstring(ctx, DUK_INVALID_INDEX, &sz);
	printf("string: %s (%d)\n", p, (int) sz);
	return 0;
}

void test(duk_context *ctx) {
	int rc;

	rc = duk_safe_call(ctx, test_1, 0, 1);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_2, 0, 1);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_3, 0, 1);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_4, 0, 1);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);
}

