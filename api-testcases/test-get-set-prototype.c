/*
 *  Testcase for object prototype manipulation
 *
 *  Test also prototype loops.  The Ecmascript primitives like Object.create()
 *  and Object.setPrototypeOf() guard against creating a prototype loop (as
 *  required by the specification) but no such safeguard is used for the C API.
 *  A prototype loop is expected to be terminated by a sanity limit inside
 *  Duktape which is explicitly implemented for all prototype traversals.
 *  Even so, user code is expected to never create a prototype loop on purpose.
 */

/*===
*** test_1 (duk_safe_call)
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
==> rc=0, result='undefined'
*** test_3 (duk_safe_call)
set obj0 prototype to obj1
set obj1 prototype to obj0
obj0.foo=123
obj0.bar=123
==> rc=1, result='Error: prototype chain limit'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	(void) ctx;
	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	(void) ctx;
	return 0;
}

static duk_ret_t test_3(duk_context *ctx) {
	duk_push_object(ctx);
	duk_push_int(ctx, 123);
	duk_put_prop_string(ctx, -2, "foo");

	duk_push_object(ctx);
	duk_push_int(ctx, 123);
	duk_put_prop_string(ctx, -2, "bar");

	/* Set object at index 0 and index 1 to use each other as their
	 * prototype and check that Duktape sanity bails out for a prototype
	 * lookup.
	 *
	 * NOTE: User code should always avoid creating prototype loops!
	 */

	printf("set obj0 prototype to obj1\n");
	duk_dup(ctx, 0);
	duk_set_prototype(ctx, 1);

	printf("set obj1 prototype to obj0\n");
	duk_dup(ctx, 1);
	duk_set_prototype(ctx, 0);

	/* For existing property, prototype loop has no impact. */

	duk_get_prop_string(ctx, 0, "foo");
	printf("obj0.foo=%s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	duk_get_prop_string(ctx, 0, "bar");
	printf("obj0.bar=%s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* Non-existent property causes the prototype loop to be traversed
	 * until Duktape hits a sanity limit.
	 */

	duk_get_prop_string(ctx, 0, "nonexistent");
	printf("obj0.foo=%s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
}
