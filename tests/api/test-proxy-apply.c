/*===
*** test_passthrough (duk_safe_call)
my_function called
argument: 123
ret=234
final top: 0
==> rc=0, result='undefined'
*** test_trap (duk_safe_call)
my_apply trap called
ret=345
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t my_function(duk_context *ctx) {
	printf("my_function called\n");
	printf("argument: %s\n", duk_to_string(ctx, 0));
	duk_push_uint(ctx, 234);
	return 1;
}

static duk_ret_t my_apply_trap(duk_context *ctx) {
	printf("my_apply trap called\n");
	duk_push_uint(ctx, 345);
	return 1;
}

static duk_ret_t test_passthrough(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_c_function(ctx, my_function, 1 /*nargs*/);  /* target */
	duk_push_object(ctx);  /* handler */

	duk_push_proxy(ctx, 0);

	duk_push_uint(ctx, 123);
	duk_call(ctx, 1);

	printf("ret=%s\n", duk_to_string(ctx, -1));

	duk_pop(ctx);  /* call result */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_trap(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_c_function(ctx, my_function, 1 /*nargs*/);  /* target */
	duk_push_object(ctx);  /* handler */

	duk_push_c_function(ctx, my_apply_trap, 3 /*nargs*/);
	duk_put_prop_string(ctx, -2, "apply");

	duk_push_proxy(ctx, 0);

	duk_push_uint(ctx, 123);
	duk_call(ctx, 1);

	printf("ret=%s\n", duk_to_string(ctx, -1));

	duk_pop(ctx);  /* call result */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_passthrough);
	TEST_SAFE_CALL(test_trap);
}
