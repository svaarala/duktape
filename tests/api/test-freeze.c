/*===
*** test_basic (duk_safe_call)
top before: 1
top after: 1
true
true
false
final top: 1
==> rc=0, result='undefined'
*** test_invalid_index (duk_safe_call)
==> rc=1, result='RangeError: invalid stack index -1'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_object(ctx);                           /* [ ... obj ] */
	duk_push_int(ctx, 42);                          /* [ ... obj 42 ] */
	duk_put_prop_string(ctx, -2, "meaningOfLife");  /* [ ... obj ] */

	printf("top before: %ld\n", (long) duk_get_top(ctx));
	duk_freeze(ctx, -1);
	printf("top after: %ld\n", (long) duk_get_top(ctx));

	duk_eval_string(ctx,
		"(function (v) {\n"
		"    print(Object.isSealed(v));\n"
		"    print(Object.isFrozen(v));\n"
		"    print(Object.isExtensible(v));\n"
		"})");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_invalid_index(duk_context *ctx, void *udata) {
	(void) udata;

	duk_freeze(ctx, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_invalid_index);
}
