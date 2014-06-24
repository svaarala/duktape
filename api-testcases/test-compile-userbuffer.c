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
	const char *src1 = "print('program code'); 123";
	const char *src2 = "print('program code'); 123; obj={";

	/* Normal compile */
	duk_compile_userbuffer(ctx, 0, src1, strlen(src1));
	duk_call(ctx, 0);
	printf("return value is: '%s'\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	/* Protected compile, success */
	rc = duk_pcompile_userbuffer(ctx, 0, src1, strlen(src1));
	printf("compile rc=%d\n", rc);
	duk_call(ctx, 0);
	printf("return value is: '%s'\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	/* Protected compile, syntax error */
	rc = duk_pcompile_userbuffer(ctx, 0, src2, strlen(src2));
	printf("compile rc=%d -> %s\n", rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	printf("top: %d\n", duk_get_top(ctx));
}
