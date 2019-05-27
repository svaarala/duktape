/*===
*** test_1 (duk_safe_call)
top: 0
duk_to_stacktrace: 'Error: aiee
    at err (eval:1)
    at eval (eval:1) preventsyield'
final top: 0
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
top: 0
duk_to_stacktrace: '4'
final top: 0
==> rc=0, result='undefined'
*** test_3 (duk_safe_call)
top: 0
duk_to_stacktrace: '[object Object]'
final top: 0
==> rc=0, result='undefined'
*** test_4 (duk_safe_call)
top: 0
duk_to_stacktrace: 'oops'
final top: 0
==> rc=0, result='undefined'
*** test_5 (duk_safe_call)
top: 0
duk_to_stacktrace: '[object Object]'
final top: 0
==> rc=0, result='undefined'
*** test_6 (duk_safe_call)
get 1
==> rc=1, result='Error: aiee 2'
*** test_7 (duk_safe_call)
get 1
==> rc=1, result='Error: aiee 2'
===*/

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	/* Normal case: normal error throw. */
	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_peval_string(ctx, "(function err() { throw new Error('aiee'); })();");
	printf("duk_to_stacktrace: '%s'\n", duk_to_stacktrace(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_2(duk_context *ctx, void *udata) {
	(void) udata;

	/* Uncommon case: coercion argument is not an Error (or even an object). */
	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_peval_string(ctx, "2+2");
	printf("duk_to_stacktrace: '%s'\n", duk_to_stacktrace(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_3(duk_context *ctx, void *udata) {
	(void) udata;

	/* Uncommon case: coercion argument is an Object without .stack. */
	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_eval_string(ctx, "({ foo: 'bar' })");
	printf("duk_to_stacktrace: '%s'\n", duk_to_stacktrace(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_4(duk_context *ctx, void *udata) {
	(void) udata;

	/* Uncommon case: coercion argument is an Object with .stack, but not
	 * an Error.
	 */
	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_eval_string(ctx, "({ stack: 'oops' })");
	printf("duk_to_stacktrace: '%s'\n", duk_to_stacktrace(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_5(duk_context *ctx, void *udata) {
	(void) udata;

	/* Uncommon case: coercion argument is an Object with .stack, but not
	 * a string.
	 */
	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_eval_string(ctx, "({ stack: 123 })");
	printf("duk_to_stacktrace: '%s'\n", duk_to_stacktrace(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_6(duk_context *ctx, void *udata) {
	(void) udata;

	/* Uncommon case: coercion fails once.  The non-safe variant passes
	 * the initial error through as is.
	 */
	duk_peval_string(ctx, "var err = new Error('aiee 1'); Object.defineProperty(err, 'stack', {\n"
	                      "    get: function() { print('get 1'); throw new Error('aiee 2'); }\n"
	                      "}); throw err;\n");
	printf("duk_to_stacktrace: '%s'\n", duk_to_stacktrace(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_7(duk_context *ctx, void *udata) {
	(void) udata;

	/* Uncommon case: coercion fails twice.  The non-safe variant passes
	 * the initial error through as is.
	 */
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
	printf("duk_to_stacktrace: '%s'\n", duk_to_stacktrace(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
	TEST_SAFE_CALL(test_5);
	TEST_SAFE_CALL(test_6);
	TEST_SAFE_CALL(test_7);
}
