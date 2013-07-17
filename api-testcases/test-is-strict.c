/*===
outside: 0
inside: 1
===*/

int my_func(duk_context *ctx) {
	printf("inside: %d\n", duk_is_strict_call(ctx));
	return 0;
}

void test(duk_context *ctx) {
	/* The context has no active function calls initially,
	 * so duk_is_strict_call() returns zero.  Whenever a
	 * Duktape/C function call is running, it returns 1
	 * because all Duktape/C function calls are now strict.
	 */

	printf("outside: %d\n", duk_is_strict_call(ctx));

	duk_push_new_c_function(ctx, my_func, 0);
	duk_call(ctx, 0);
}

