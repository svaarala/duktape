/*
 *  Example of accessing Duktape's internal property keys from C code
 *  (not recommended but possible).
 *
 *  There is no way to prevent this through access control or such.
 *  C code must necessarily be trusted anyway, because it has full
 *  memory access.
 */

/*===
*** test_1 (duk_safe_call)
Date._Value: 123456
final top: 2
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	duk_eval_string(ctx, "new Date(123456)");
	duk_push_string(ctx, "\xffValue");
	duk_get_prop(ctx, -2);
	printf("Date._Value: %s\n", duk_safe_to_string(ctx, -1));
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
