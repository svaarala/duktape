/*===
program code
return value is: '123'
compile rc=0
program code
return value is: '123'
compile rc=1 -> SyntaxError: invalid object literal (line 1)
top: 0
===*/

void test(duk_context *ctx) {
	int rc;

	/* Normal compile */
	duk_compile_string(ctx, 0, "print('program code'); 123");
	duk_call(ctx, 0);
	printf("return value is: '%s'\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	/* Protected compile, success */
	rc = duk_pcompile_string(ctx, 0, "print('program code'); 123");
	printf("compile rc=%d\n", rc);
	duk_call(ctx, 0);
	printf("return value is: '%s'\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	/* Protected compile, syntax error */
	rc = duk_pcompile_string(ctx, 0, "print('program code'); 123; obj={");
	printf("compile rc=%d -> %s\n", rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	printf("top: %d\n", duk_get_top(ctx));
}

