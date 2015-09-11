/*
 *  Bug in duk_is_primitive() prior to Duktape 1.3.0: returns 1 for invalid
 *  index.
 *
 *  https://github.com/svaarala/duktape/issues/337
 */

/*===
*** test_1 (duk_safe_call)
0
final top: 0
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
0
final top: 0
==> rc=0, result='undefined'
*** test_3 (duk_safe_call)
1
0
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	printf("%ld\n", (long) duk_is_primitive(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	printf("%ld\n", (long) duk_is_primitive(ctx, DUK_INVALID_INDEX));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_3(duk_context *ctx) {
	duk_push_null(ctx);

	printf("%ld\n", (long) duk_is_primitive(ctx, 0));  /* valid */
	printf("%ld\n", (long) duk_is_primitive(ctx, 1));  /* invalid */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
}
