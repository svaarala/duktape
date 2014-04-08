/*===
duk_is_constructor_call: 0
duk_is_constructor_call: 1
rc=0, ret=undefined
===*/

int my_func(duk_context *ctx) {
	printf("duk_is_constructor_call: %d\n", duk_is_constructor_call(ctx));
	return 0;
}

int test_raw(duk_context *ctx) {
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
	int rc;

	rc = duk_safe_call(ctx, test_raw, 0, 1);
	printf("rc=%d, ret=%s\n", rc, duk_to_string(ctx, -1));
}

