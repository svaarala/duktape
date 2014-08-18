/*
 *  When eval'ing code using the Duktape API (duk_eval() and friends), the
 *  eval code is evaluated in non-strict mode by default.  If the eval code
 *  contains a "use strict" directive, then it is of course evaluated in
 *  strict mode.
 *
 *  This behavior is a change from Duktape 0.11.0 where the strictness of the
 *  current context would be inherited into eval code.  Because the Duktape
 *  context is non-strict when no Duktape/C calls active and strict when a
 *  Duktape/C call is active, eval() code strictness would also depend on
 *  whether duk_eval() was called inside or outside of a Duktape/C activation.
 *  This is quite confusing.  One practical issue is that strict eval code
 *  gets a fresh variable declaration environment and the following will
 *  behave differently:
 *
 *      var foo = 'bar';
 *
 *  In non-strict mode this establishes a 'foo' property in the global object.
 *  In strict mode it declares a new 'foo' variable in a fresh, temporary
 *  lexical environment and doesn't have an effect on the global object.
 *
 *  The strictness behavior was changed in Duktape 0.12.0 so that duk_eval()
 *  always compiles code in non-strict mode so that it's possible to eval
 *  non-strict code inside a Duktape/C activation.  This matches the behavior
 *  of duk_compile() which also doesn't "inherit" strictness from the Duktape/C
 *  context.  Further, in Duktape 0.12.0 and onwards the Duktape/C context is
 *  always considered strict to minimize confusion.
 */

/*===
*** test_1 (duk_safe_call)
context is strict: 1
test_1 evalcode, typeof Math: object
global.foo1=bar
final top: 0
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
context is strict: 1
context is strict: 1
test_2 evalcode, typeof Math: object
global.foo2=bar
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	const char *str;

	/* Eval happens outside of a Duktape/C activation.  The eval code was
	 * executed in non-strict mode also in Duktape 0.11.0 and prior.
	 */

	printf("context is strict: %d\n", duk_is_strict_call(ctx));
	duk_eval_string_noresult(ctx, "print('test_1 evalcode, typeof Math:', typeof Math); var foo1 = 'bar';");
	duk_push_global_object(ctx);
	duk_get_prop_string(ctx, -1, "foo1");
	str = duk_get_string(ctx, -1);
	printf("global.foo1=%s\n", str ? str : "NULL");
	duk_pop_2(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_2_inner(duk_context *ctx) {
	/* Eval happens inside a Duktape/C activation, so we're currently in
	 * in strict mode.  In Duktape 0.11.0 and prior, the 'foo2' declaration
	 * would go into a temporary lexical environment and would not be visible
	 * in the global object.  The code can still read (and write) properties
	 * of the global object.
	 */

	printf("context is strict: %d\n", duk_is_strict_call(ctx));
	duk_eval_string_noresult(ctx, "print('test_2 evalcode, typeof Math:', typeof Math); var foo2 = 'bar';");
	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	const char *str;

	printf("context is strict: %d\n", duk_is_strict_call(ctx));

	duk_push_c_function(ctx, test_2_inner, 0);
	duk_call(ctx, 0);
	duk_pop(ctx);

	duk_push_global_object(ctx);
	duk_get_prop_string(ctx, -1, "foo2");
	str = duk_get_string(ctx, -1);
	printf("global.foo2=%s\n", str ? str : "NULL");
	duk_pop_2(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
}
