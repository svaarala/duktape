/*===
*** test_basic (duk_safe_call)
top: 0
top: 1
ret: 1
encoded: foo%20bar
top: 0
top: 1
ret: 0
undefined
top: 0
top: 1
ret: 1
nulval
top: 0
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	duk_bool_t ret;

	(void) udata;

	duk_eval_string_noresult(ctx, "new Function('return this')()['nul\\u0000key'] = 'nulval';");

	printf("top: %ld\n", (long) duk_get_top(ctx));
	ret = duk_get_global_string(ctx, "encodeURIComponent" "\x00" "ignored");
	printf("top: %ld\n", (long) duk_get_top(ctx));
	printf("ret: %ld\n", (long) ret);
	duk_push_string(ctx, "foo bar");
	duk_call(ctx, 1);
	printf("encoded: %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	ret = duk_get_global_string(ctx, "doesNotExist");
	printf("top: %ld\n", (long) duk_get_top(ctx));
	printf("ret: %ld\n", (long) ret);
	printf("%s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	ret = duk_get_global_lstring(ctx, "nul" "\x00" "keyx", 7);
	printf("top: %ld\n", (long) duk_get_top(ctx));
	printf("ret: %ld\n", (long) ret);
	printf("%s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
