/*===
*** test_string (duk_safe_call)
program code
return value is: '123'
myFile.js
return value is: '234'
compile rc=0
program code
return value is: '123'
compile rc=0
myFile.js
return value is: '234'
compile rc=1 -> SyntaxError: invalid object literal (line 1, end of input)
top: 0
==> rc=0, result='undefined'
*** test_lstring (duk_safe_call)
program code
return value is: '123'
myFile.js
return value is: '234'
compile rc=0
program code
return value is: '123'
compile rc=0
myFile.js
return value is: '234'
compile rc=1 -> SyntaxError: invalid object literal (line 1, end of input)
top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_string(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	/* Normal compile */
	duk_compile_string(ctx, 0, "print('program code'); 123");
	duk_call(ctx, 0);
	printf("return value is: '%s'\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	/* Normal compile with explicit filename */
	duk_push_string(ctx, "myFile.js");
	duk_compile_string_filename(ctx, 0, "print(Duktape.act(-2).function.fileName); 234");
	duk_call(ctx, 0);
	printf("return value is: '%s'\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	/* Protected compile, success */
	rc = duk_pcompile_string(ctx, 0, "print('program code'); 123");
	printf("compile rc=%ld\n", (long) rc);
	duk_call(ctx, 0);
	printf("return value is: '%s'\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	/* Protected compile with explicit filename, success */
	duk_push_string(ctx, "myFile.js");
	rc = duk_pcompile_string_filename(ctx, 0, "print(Duktape.act(-2).function.fileName); 234");
	printf("compile rc=%ld\n", (long) rc);
	duk_call(ctx, 0);
	printf("return value is: '%s'\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	/* Protected compile, syntax error */
	rc = duk_pcompile_string(ctx, 0, "print('program code'); 123; obj={");
	printf("compile rc=%ld -> %s\n", (long) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_lstring(duk_context *ctx, void *udata) {
	duk_int_t rc;
	const char *src1 = "print('program code'); 123@";
	const char *src2 = "print(Duktape.act(-2).function.fileName); 234@";
	const char *src3 = "print('program code'); 123; obj={@";

	(void) udata;

	/* Normal compile */
	duk_compile_lstring(ctx, 0, src1, strlen(src1) - 1);
	duk_call(ctx, 0);
	printf("return value is: '%s'\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	/* Normal compile with explicit filename */
	duk_push_string(ctx, "myFile.js");
	duk_compile_lstring_filename(ctx, 0, src2, strlen(src2) - 1);
	duk_call(ctx, 0);
	printf("return value is: '%s'\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	/* Protected compile, success */
	rc = duk_pcompile_lstring(ctx, 0, src1, strlen(src1) - 1);
	printf("compile rc=%ld\n", (long) rc);
	duk_call(ctx, 0);
	printf("return value is: '%s'\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	/* Protected compile with explicit filename, success */
	duk_push_string(ctx, "myFile.js");
	rc = duk_pcompile_lstring_filename(ctx, 0, src2, strlen(src2) - 1);
	printf("compile rc=%ld\n", (long) rc);
	duk_call(ctx, 0);
	printf("return value is: '%s'\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	/* Protected compile, syntax error */
	rc = duk_pcompile_lstring(ctx, 0, src3, strlen(src3) - 1);
	printf("compile rc=%ld -> %s\n", (long) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_string);
	TEST_SAFE_CALL(test_lstring);
}
