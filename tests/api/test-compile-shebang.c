/*
 *  Shebang comment, only allowed when explicitly requested.
 */

/*===
*** test_not_allowed (duk_safe_call)
compile rc: 1
top: 1
call rc: 1
top: 1
==> rc=0, result='undefined'
*** test_allowed (duk_safe_call)
compile rc: 0
top: 1
allowed
call rc: 0
top: 1
==> rc=0, result='undefined'
*** test_allowed_eval (duk_safe_call)
compile rc: 0
top: 1
allowed
call rc: 0
top: 1
==> rc=0, result='undefined'
*** test_not_first_column (duk_safe_call)
==> rc=1, result='SyntaxError: invalid token (line 1)'
*** test_not_first_line (duk_safe_call)
==> rc=1, result='SyntaxError: invalid token (line 2)'
===*/

static duk_ret_t test_not_allowed(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	duk_push_string(ctx, "#!foo\nprint('not allowed');\n");
	duk_push_string(ctx, "test.js");
	rc = duk_pcompile(ctx, 0);
	printf("compile rc: %ld\n", (long) rc);
	printf("top: %ld\n", (long) duk_get_top(ctx));
	rc = duk_pcall(ctx, 0);
	printf("call rc: %ld\n", (long) rc);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	return 0;
}

static duk_ret_t test_allowed(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	duk_push_string(ctx, "#!foo\nprint('allowed');\n");
	duk_push_string(ctx, "test.js");
	rc = duk_pcompile(ctx, DUK_COMPILE_SHEBANG);
	printf("compile rc: %ld\n", (long) rc);
	printf("top: %ld\n", (long) duk_get_top(ctx));
	rc = duk_pcall(ctx, 0);
	printf("call rc: %ld\n", (long) rc);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	return 0;
}

static duk_ret_t test_allowed_eval(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	duk_push_string(ctx, "#!foo\nprint('allowed');\n");
	duk_push_string(ctx, "test.js");
	rc = duk_pcompile(ctx, DUK_COMPILE_EVAL | DUK_COMPILE_SHEBANG);
	printf("compile rc: %ld\n", (long) rc);
	printf("top: %ld\n", (long) duk_get_top(ctx));
	rc = duk_pcall(ctx, 0);
	printf("call rc: %ld\n", (long) rc);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	return 0;
}

static duk_ret_t test_not_first_column(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_string(ctx, " #!foo\nprint('not allowed, must be first column on first line');\n");
	duk_push_string(ctx, "test.js");
	duk_compile(ctx, DUK_COMPILE_SHEBANG);
	printf("should not succeed\n");
	printf("top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_not_first_line(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	duk_push_string(ctx, "\n#!foo\nprint('not allowed, must be first column on first line');\n");
	duk_push_string(ctx, "test.js");
	duk_compile(ctx, DUK_COMPILE_SHEBANG);
	printf("should not succeed\n");
	printf("top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_not_allowed);
	TEST_SAFE_CALL(test_allowed);
	TEST_SAFE_CALL(test_allowed_eval);
	TEST_SAFE_CALL(test_not_first_column);
	TEST_SAFE_CALL(test_not_first_line);
}
