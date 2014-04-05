/*===
Hello world!
return value is: 123.000000
result is: 'TESTSTRING'
Hello world!
return value is: 123 (rc=0)
return value is: Error: eval error (rc=1)
return value is: SyntaxError: invalid object literal (line 1) (rc=1)
top: 0
===*/

void test(duk_context *ctx) {
	int rc;

	duk_eval_string(ctx, "print('Hello world!'); 123;");
	printf("return value is: %lf\n", duk_get_number(ctx, -1));
	duk_pop(ctx);

	duk_eval_string(ctx, "'testString'.toUpperCase()");
	printf("result is: '%s'\n", duk_get_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_peval_string(ctx, "print('Hello world!'); 123;");
	printf("return value is: %s (rc=%d)\n", duk_safe_to_string(ctx, -1), rc);
	duk_pop(ctx);

	rc = duk_peval_string(ctx, "throw new Error('eval error');");
	printf("return value is: %s (rc=%d)\n", duk_safe_to_string(ctx, -1), rc);
	duk_pop(ctx);

	rc = duk_peval_string(ctx, "throw new Error('eval error'); obj = {");
	printf("return value is: %s (rc=%d)\n", duk_safe_to_string(ctx, -1), rc);
	duk_pop(ctx);

	printf("top: %d\n", duk_get_top(ctx));
}

