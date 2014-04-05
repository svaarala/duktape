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

int test_1(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "");
	printf("string: %s\n", duk_require_string(ctx, 0));
	printf("string: %s\n", duk_require_string(ctx, 1));
	return 0;
}

int test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_null(ctx);
	printf("string: %s\n", duk_require_string(ctx, 0));
	return 0;
}

int test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);
	printf("string: %s\n", duk_require_string(ctx, 0));
	return 0;
}

int test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);
	printf("string: %s\n", duk_require_string(ctx, DUK_INVALID_INDEX));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
}
