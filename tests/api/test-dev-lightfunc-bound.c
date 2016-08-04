/*
 *  Lightfunc cannot be bound, but can appear in a bound function chain as
 *  the final non-bound function.  This may happen for both constuctor and
 *  non-constructor call.
 */

/*===
*** test_normal_call (duk_safe_call)
lightfunc called, constructor call: 0
argument 1: 123
argument 2: 234
return value: dummy
lightfunc called, constructor call: 0
argument 1: 1001
argument 2: 2002
return value: dummy
final top: 1
==> rc=0, result='undefined'
*** test_constructor_call (duk_safe_call)
lightfunc called, constructor call: 1
argument 1: 123
argument 2: 234
return value: [object Object]
lightfunc called, constructor call: 1
argument 1: 1001
argument 2: 2002
return value: [object Object]
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t my_lightfunc(duk_context *ctx) {
	printf("lightfunc called, constructor call: %d\n", (int) duk_is_constructor_call(ctx));
	printf("argument 1: %s\n", duk_to_string(ctx, 0));
	printf("argument 2: %s\n", duk_to_string(ctx, 1));
	duk_push_string(ctx, "dummy");
	return 1;
}

static duk_ret_t test_normal_call(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_c_lightfunc(ctx, my_lightfunc, 2, 0, 0);

	/* Call directly. */
	duk_dup(ctx, -1);
	duk_push_uint(ctx, 123);
	duk_push_uint(ctx, 234);
	duk_call(ctx, 2);
	printf("return value: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* Call via bound function. */
	duk_eval_string(ctx,
		"(function (v) {\n"
		"    return v.bind(null, 1001, 2002);\n"
		"})");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);  /* -> [ lfunc bound ] */
	duk_call(ctx, 0);
	printf("return value: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_constructor_call(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_c_lightfunc(ctx, my_lightfunc, 2, 0, 0);

	/* Call directly. */
	duk_dup(ctx, -1);
	duk_push_uint(ctx, 123);
	duk_push_uint(ctx, 234);
	duk_new(ctx, 2);
	printf("return value: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* Call via bound function. */
	duk_eval_string(ctx,
		"(function (v) {\n"
		"    return v.bind(null, 1001, 2002);\n"
		"})");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);  /* -> [ lfunc bound ] */
	duk_new(ctx, 0);
	printf("return value: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_normal_call);
	TEST_SAFE_CALL(test_constructor_call);
}
