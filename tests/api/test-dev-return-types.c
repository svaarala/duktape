/*
 *  Supporting testcase for test-dev-return-types.js: cover return types
 *  which can't be exercised from Ecmascript code.
 */

/*===
*** test_basic_implicit (duk_safe_call)
inside func
result: undefined
final top: 0
==> rc=0, result='undefined'
*** test_basic_explicit (duk_safe_call)
inside func
result: 123
final top: 0
==> rc=0, result='undefined'
*** test_endfin_return (duk_safe_call)
finally
result: 321
final top: 0
==> rc=0, result='undefined'
inside func
result: undefined
inside func
result: undefined
inside func
result: 123
inside func
result: 123
finally
result: 321
finally
result: 321
final top: 0
===*/

/* Normal call from a duk_safe_call() wrapper, implicit return value. */
static duk_ret_t test_basic_implicit(duk_context *ctx) {
	duk_eval_string(ctx,
		"(function () {\n"
		"    print('inside func');\n"
		"})");
	duk_call(ctx, 0);
	printf("result: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Normal call from a duk_safe_call() wrapper, explicit return value. */
static duk_ret_t test_basic_explicit(duk_context *ctx) {
	duk_eval_string(ctx,
		"(function () {\n"
		"    print('inside func');\n"
		"    return 123;\n"
		"})");
	duk_call(ctx, 0);
	printf("result: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Ecmascript finally captures return and the return is propagated
 * onwards after finally finishes.
 */
static duk_ret_t test_endfin_return(duk_context *ctx) {
	duk_eval_string(ctx,
		"(function () {\n"
		"    try {\n"
		"        return 321;\n"
		"    } finally {\n"
		"        print('finally');\n"
		"    }\n"
		"    print('never here');\n"
		"})");
	duk_call(ctx, 0);
	printf("result: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic_implicit);
	TEST_SAFE_CALL(test_basic_explicit);
	TEST_SAFE_CALL(test_endfin_return);

	/* Top level unprotected call + return with implicit value. */
	duk_eval_string(ctx,
		"(function () {\n"
		"    print('inside func');\n"
		"})");
	duk_call(ctx, 0);
	printf("result: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* Top level protected call + return with explicit value. */
	duk_eval_string(ctx,
		"(function () {\n"
		"    print('inside func');\n"
		"})");
	(void) duk_pcall(ctx, 0);
	printf("result: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* Top level unprotected call + return with explicit value. */
	duk_eval_string(ctx,
		"(function () {\n"
		"    print('inside func');\n"
		"    return 123;\n"
		"})");
	duk_call(ctx, 0);
	printf("result: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* Top level protected call + return with explicit value. */
	duk_eval_string(ctx,
		"(function () {\n"
		"    print('inside func');\n"
		"    return 123;\n"
		"})");
	duk_pcall(ctx, 0);
	printf("result: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* ENDFIN + RETURN case directly from top level, unprotected call. */
	duk_eval_string(ctx,
		"(function () {\n"
		"    try {\n"
		"        return 321;\n"
		"    } finally {\n"
		"        print('finally');\n"
		"    }\n"
		"    print('never here');\n"
		"})");
	duk_call(ctx, 0);
	printf("result: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/* ENDFIN + RETURN case directly from top level, protected call. */
	duk_eval_string(ctx,
		"(function () {\n"
		"    try {\n"
		"        return 321;\n"
		"    } finally {\n"
		"        print('finally');\n"
		"    }\n"
		"    print('never here');\n"
		"})");
	duk_pcall(ctx, 0);
	printf("result: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
}
