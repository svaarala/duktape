/*===
*** test_1 (duk_safe_call)
top: 0
duk_safe_to_stacktrace: 'Error: aiee
    at err (eval:1)
    at eval (eval:1) preventsyield'
top: 0
duk_safe_to_stacktrace: '4'
top: 0
duk_safe_to_stacktrace: '[object Object]'
top: 0
duk_safe_to_stacktrace: 'oops'
top: 0
duk_safe_to_stacktrace: '[object Object]'
get 1
duk_safe_to_stacktrace: 'Error: aiee 2
    at [anon] (eval:2) preventsyield'
get 1
get 2
duk_safe_to_stacktrace: 'Error'
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	/* Normal case: normal error throw. */
	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_peval_string(ctx, "(function err() { throw new Error('aiee'); })();");
	printf("duk_safe_to_stacktrace: '%s'\n", duk_safe_to_stacktrace(ctx, -1));
	duk_pop(ctx);

	/* Uncommon case: coercion argument is not an Error (or even an object). */
	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_peval_string(ctx, "2+2");
	printf("duk_safe_to_stacktrace: '%s'\n", duk_safe_to_stacktrace(ctx, -1));
	duk_pop(ctx);

	/* Uncommon case: coercion argument is an Object without .stack. */
	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_eval_string(ctx, "({ foo: 'bar' })");
	printf("duk_safe_to_stacktrace: '%s'\n", duk_safe_to_stacktrace(ctx, -1));
	duk_pop(ctx);

	/* Uncommon case: coercion argument is an Object with .stack, but not
	 * an Error.
	 */
	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_eval_string(ctx, "({ stack: 'oops' })");
	printf("duk_safe_to_stacktrace: '%s'\n", duk_safe_to_stacktrace(ctx, -1));
	duk_pop(ctx);

	/* Uncommon case: coercion argument is an Object with .stack, but not
	 * a string.
	 */
	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_eval_string(ctx, "({ stack: 123 })");
	printf("duk_safe_to_stacktrace: '%s'\n", duk_safe_to_stacktrace(ctx, -1));
	duk_pop(ctx);

	/* Uncommon case: coercion fails once. */
	duk_peval_string(ctx, "var err = new Error('aiee 1'); Object.defineProperty(err, 'stack', {\n"
	                      "    get: function() { print('get 1'); throw new Error('aiee 2'); }\n"
	                      "}); throw err;\n");
	printf("duk_safe_to_stacktrace: '%s'\n", duk_safe_to_stacktrace(ctx, -1));
	duk_pop(ctx);

	/* Uncommon case: coercion fails twice. */
	duk_peval_string(ctx, "var err = new Error('aiee 1'); Object.defineProperty(err, 'stack', {\n"
	                      "    get: function() {\n"
	                      "        print('get 1');\n"
	                      "        var e = new Error('aiee 2');\n"
	                      "        Object.defineProperty(e, 'stack', {\n"
	                      "            get: function() { print('get 2'); throw new Error('aiee 3'); }\n"
	                      "        });\n"
	                      "        throw e;\n"
	                      "    }\n"
	                      "}); throw err;\n");
	printf("duk_safe_to_stacktrace: '%s'\n", duk_safe_to_stacktrace(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
