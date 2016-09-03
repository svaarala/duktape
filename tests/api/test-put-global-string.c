/*===
*** test_basic (duk_safe_call)
top: 0
top: 0
ret: 1
1.2.3
top: 0
top: 0
ret: 1
nulval
final top: 0
==> rc=0, result='undefined'
*** test_nonwritable (duk_safe_call)
top: 0
==> rc=1, result='TypeError: not writable'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	duk_bool_t ret;

	(void) udata;

	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_push_string(ctx, "1.2.3");
	ret = duk_put_global_string(ctx, "myAppVersion" "\x00" "ignored");
	printf("top: %ld\n", (long) duk_get_top(ctx));
	printf("ret: %ld\n", (long) ret);

	duk_eval_string_noresult(ctx, "print(myAppVersion);");

	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_push_string(ctx, "nulval");
	ret = duk_put_global_lstring(ctx, "nul" "\x00" "keyx", 7);
	printf("top: %ld\n", (long) duk_get_top(ctx));
	printf("ret: %ld\n", (long) ret);

	duk_eval_string_noresult(ctx, "print(new Function('return this')()['nul\\u0000key']);");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_nonwritable(duk_context *ctx, void *udata) {
	duk_bool_t ret;

	(void) udata;

	duk_eval_string_noresult(ctx,
		"Object.defineProperty(this, 'nonWritable', "
		"{ value: 'foo', writable: false, enumerable: false, configurable: false });"
	);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_push_string(ctx, "bar");
	ret = duk_put_global_string(ctx, "nonWritable");
	printf("top: %ld\n", (long) duk_get_top(ctx));
	printf("ret: %ld\n", (long) ret);

	duk_eval_string_noresult(ctx, "print(nonWritable);");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_nonwritable);
}
