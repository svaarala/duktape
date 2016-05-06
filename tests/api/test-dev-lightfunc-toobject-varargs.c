/*
 *  ToObject() lightfunc coercion for varargs.  Handling for this case had
 *  fragile (but working) casts in Duktape 1.3.x.
 */

/*===
*** test_1 (duk_safe_call)
top: 12
top: 12
final top: 1
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
top: 2
top: 2
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t my_lightfunc(duk_context *ctx) {
	printf("top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	/* First test: lightfunc without varargs. */

	duk_push_c_lightfunc(ctx, my_lightfunc, 12 /*nargs*/, 3 /*length*/, 0 /*magic*/);

	duk_dup_top(ctx);
	duk_call(ctx, 0);
	duk_pop(ctx);

	duk_dup_top(ctx);
	duk_to_object(ctx, -1);
	duk_call(ctx, 0);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_2(duk_context *ctx, void *udata) {
	(void) udata;

	/* Second test: lightfunc with varargs. */

	duk_push_c_lightfunc(ctx, my_lightfunc, DUK_VARARGS /*nargs*/, 3 /*length*/, 0 /*magic*/);

	duk_dup_top(ctx);
	duk_push_true(ctx);
	duk_push_false(ctx);
	duk_call(ctx, 2);
	duk_pop(ctx);

	duk_dup_top(ctx);
	duk_to_object(ctx, -1);
	duk_push_true(ctx);
	duk_push_false(ctx);
	duk_call(ctx, 2);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
}
