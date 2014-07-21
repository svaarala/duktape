/*
 *  Example of using internal properties from C code.
 */

/*===
*** test_1 (duk_safe_call)
{foo:1," \xffbar":3}
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	duk_eval_string(ctx, "(function (x) { print(Duktape.enc('jx', x)); })");

	duk_push_object(ctx);

	/* Ordinary property */
	duk_push_int(ctx, 1);
	duk_put_prop_string(ctx, -2, "foo");  /* obj.foo = 1 */

	/* Internal property \xFF\xFFabc, technically enumerable (based on
	 * property attributes) but because of internal property special
	 * behavior, does not enumerate.
	 */

	duk_push_int(ctx, 2);
	duk_put_prop_string(ctx, -2, "\xff\xff" "abc");  /* obj[\xff\xffabc] = 2, internal property */

	/* Another property with invalid UTF-8 data but doesn't begin with
	 * \xFF => gets enumerated and JX prints out an approximate key.
	 */
	duk_push_int(ctx, 3);
	duk_put_prop_string(ctx, -2, " \xff" "bar");  /* obj[ \xffbar] = 3, invalid utf-8 but not an internal property */
	duk_call(ctx, 1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
