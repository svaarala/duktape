/*===
*** test_1 (duk_safe_call)
duk_is_constructor_call: 0
duk_is_constructor_call: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t my_func(duk_context *ctx) {
	printf("duk_is_constructor_call: %d\n", (int) duk_is_constructor_call(ctx));
	return 0;
}

static duk_ret_t test_1(duk_context *ctx) {
	duk_push_c_function(ctx, my_func, 0);

	duk_dup(ctx, 0);   /* -> [ func func ] */
	duk_call(ctx, 0);  /* -> [ func ret ] */
	duk_pop(ctx);      /* -> [ func ] */

	duk_dup(ctx, 0);   /* -> [ func func ] */
	duk_new(ctx, 0);   /* -> [ func ret ] */
	duk_pop(ctx);      /* -> [ func ] */

	duk_pop(ctx);

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
