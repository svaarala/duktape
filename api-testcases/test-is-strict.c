/*===
outside: 0
inside: 1
===*/

static duk_ret_t my_func(duk_context *ctx) {
	printf("inside: %d\n", (int) duk_is_strict_call(ctx));
	return 0;
}

void test(duk_context *ctx) {
	/* The context has no active function calls initially,
	 * so duk_is_strict_call() returns zero.  Whenever a
	 * Duktape/C function call is running, it returns 1
	 * because all Duktape/C function calls are now strict.
	 */

	printf("outside: %d\n", (int) duk_is_strict_call(ctx));

	duk_push_c_function(ctx, my_func, 0);
	duk_call(ctx, 0);
}

