/*===
*** test_passthrough (duk_safe_call)
my_constructor called
argument: 123
ret.foo=bar
final top: 0
==> rc=0, result='undefined'
*** test_trap (duk_safe_call)
my_construct trap called
ret.foo=bar-trap
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t my_constructor(duk_context *ctx) {
	printf("my_constructor called\n");
	printf("argument: %s\n", duk_to_string(ctx, 0));
	duk_eval_string(ctx, "({ foo: 'bar' })");
	return 1;
}

static duk_ret_t my_construct_trap(duk_context *ctx) {
	printf("my_construct trap called\n");
	duk_eval_string(ctx, "({ foo: 'bar-trap' })");
	return 1;
}

static duk_ret_t test_passthrough(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_c_function(ctx, my_constructor, 1 /*nargs*/);  /* target */
	duk_push_object(ctx);  /* handler */

	duk_push_proxy(ctx, 0);

	duk_push_uint(ctx, 123);
	duk_new(ctx, 1 /*nargs*/);

	duk_get_prop_string(ctx, -1, "foo");
	printf("ret.foo=%s\n", duk_to_string(ctx, -1));

	duk_pop_2(ctx);  /* 'foo', constructor result */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_trap(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_c_function(ctx, my_constructor, 1 /*nargs*/);  /* target */
	duk_push_object(ctx);  /* handler */

	duk_push_c_function(ctx, my_construct_trap, 3 /*nargs*/);
	duk_put_prop_string(ctx, -2, "construct");

	duk_push_proxy(ctx, 0);

	duk_push_uint(ctx, 123);
	duk_new(ctx, 1 /*nargs*/);

	duk_get_prop_string(ctx, -1, "foo");
	printf("ret.foo=%s\n", duk_to_string(ctx, -1));

	duk_pop_2(ctx);  /* 'foo', constructor result */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_passthrough);
	TEST_SAFE_CALL(test_trap);
}
