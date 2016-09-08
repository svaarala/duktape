/*
 *  Test for a bug in value stack handling (Duktape 1.5.1 and prior) where
 *  the target object and the property value are the same value stack item
 *  in duk_put_prop_string(), duk_put_prop_lstring(), or duk_put_prop_index().
 *  Handling for duk_put_prop() doesn't have the issue.
 */

/*===
*** test_1 (duk_safe_call)
[object Object]
true false
true false
final top: 2
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
[object Object]
true false
true false
final top: 2
==> rc=0, result='undefined'
*** test_3 (duk_safe_call)
[object Object]
true false
true false
final top: 2
==> rc=0, result='undefined'
*** test_4 (duk_safe_call)
[object Object]
false true
false true
final top: 2
==> rc=0, result='undefined'
*** test_5 (duk_safe_call)
[object Object]
true false
true false
final top: 2
==> rc=0, result='undefined'
===*/

static void dump_result(duk_context *ctx) {
	duk_eval_string(ctx,
		"(function (v) {\n"
		"    print(v);\n"
		"    print('example' in v, '123' in v);\n"
		"    print(v.example === v, v['123'] === v);\n"
		"})\n");
	duk_dup(ctx, 0);
	duk_call(ctx, 1);
}

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) duk_push_object(ctx);

	/* This is the ordinary case where the target object and property value
	 * are distinct value stack entries.
	 */
	duk_dup_top(ctx);
	duk_put_prop_string(ctx, -2, "example");

	dump_result(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_2(duk_context *ctx, void *udata) {
	(void) duk_push_object(ctx);

	/* Here the target object and the property value are the same object.
	 * Technically this should still work but in Duktape 1.5.1 and prior
	 * this doesn't work.  The result is:
	 *
	 *     TypeError: cannot write property 'example' of 'example'
	 *
	 * The root cause is that duk_put_prop_string() pushes the key, swaps
	 * the key and the value, and then calls duk_put_prop() with the
	 * normalized index of the object intact.  This is incorrect:
	 * if the value was also the target, it's now out of position.
	 */
	duk_dup_top(ctx);
	duk_put_prop_string(ctx, -1, "example");

	dump_result(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_3(duk_context *ctx, void *udata) {
	(void) duk_push_object(ctx);

	duk_dup_top(ctx);
	duk_put_prop_lstring(ctx, -1, "example", 7);

	dump_result(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_4(duk_context *ctx, void *udata) {
	(void) duk_push_object(ctx);

	duk_dup_top(ctx);
	duk_put_prop_index(ctx, -1, 123);

	dump_result(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_5(duk_context *ctx, void *udata) {
	(void) duk_push_object(ctx);

	/* The issue doesn't manifest in duk_put_prop(). */

	duk_push_string(ctx, "example");
	duk_dup(ctx, 0);
	duk_put_prop(ctx, -1);

	dump_result(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
	TEST_SAFE_CALL(test_5);
}
