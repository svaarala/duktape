/*===
*** test_basic (duk_safe_call)
1
0
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	(void) udata;

	duk_eval_string(ctx, "new Proxy([1, 2, 3], {});");
	printf("%ld\n", (long) duk_is_array(ctx, -1));
	duk_pop(ctx);

#if 0
	duk_eval_string(ctx, "new Proxy(new Proxy([1, 2, 3], {}), {});");
	printf("%ld\n", (long) duk_is_array(ctx, -1));
	duk_pop(ctx);
#endif

	duk_eval_string(ctx, "new Proxy({ '1': 'foo' }, {});");
	printf("%ld\n", (long) duk_is_array(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
