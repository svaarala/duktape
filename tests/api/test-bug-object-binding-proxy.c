/*
 *  Counterpart to test-bug-object-binding-proxy.js: demonstrate bug in
 *  Duktape 1.2.x where a Proxy doesn't work as a global object replacement
 *  as expected.  See: https://github.com/svaarala/duktape/issues/221.
 */

/*===
*** test_1 (duk_safe_call)
get print
hello using this.print
rc=0: undefined
has print
get print
hello using print
rc=0: undefined
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	duk_int_t rc;

	duk_eval_string(ctx,
		"new Proxy(new Function('return this')() /* global object */, {\n"
		"    get: function (targ, key, recv) {\n"
		"        targ.print('get ' + key); /* careful to call print() directly without invoking Proxy! */\n"
		"        return targ[key];\n"
		"    },\n"
		"    has: function (targ, key) {\n"
		"        targ.print('has ' + key);\n"
		"        return key in targ;\n"
		"    }\n"
		"});\n"
	);
	duk_set_global_object(ctx);

	/* This worked in Duktape 1.2.x because this.print goes through a
	 * property lookup with proper Proxy support.
	 */
	rc = duk_peval_string(ctx, "this.print('hello using this.print');");
	printf("rc=%ld: %s\n", (long) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* This didn't work because print is looked up using a slow path
	 * GETVAR which goes through variable lookup code which didn't
	 * support Proxy properly.
	 */
	rc = duk_peval_string(ctx, "print('hello using print');");
	printf("rc=%ld: %s\n", (long) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
