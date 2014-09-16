/*===
*** test_1 (duk_safe_call)
string: foo
string:
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
==> rc=1, result='TypeError: not string'
*** test_3 (duk_safe_call)
==> rc=1, result='TypeError: not string'
*** test_4 (duk_safe_call)
==> rc=1, result='TypeError: not string'
===*/

static void dump_string(const char *p) {
	printf("string:%s%s\n", (strlen(p) == 0 ? "" : " "), p);
}

static duk_ret_t test_1(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "");
	dump_string(duk_require_string(ctx, 0));
	dump_string(duk_require_string(ctx, 1));
	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_null(ctx);
	dump_string(duk_require_string(ctx, 0));
	return 0;
}

static duk_ret_t test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);
	dump_string(duk_require_string(ctx, 0));
	return 0;
}

static duk_ret_t test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);
	dump_string(duk_require_string(ctx, DUK_INVALID_INDEX));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
}
