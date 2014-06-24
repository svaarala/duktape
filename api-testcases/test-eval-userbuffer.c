/*===
Hello world!
return value is: 123.000000
result is: 'TESTSTRING'
Hello world!
return value is: 123 (rc=0)
return value is: Error: eval error (rc=1)
return value is: SyntaxError: invalid object literal (line 1) (rc=1)
top=0
Hello world!
top=0
Hello world!
no result, rc=0
top=0
no result, rc=1
top: 0
===*/

void test(duk_context *ctx) {
	int rc;
	const char *src1 = "print('Hello world!'); 123;";
	const char *src2 = "'testString'.toUpperCase()";
	const char *src3 = "throw new Error('eval error');";
	const char *src4 = "throw new Error('eval error'); obj = {";
	const char *src5 = "print('Hello world!'); obj = {";

	duk_eval_userbuffer(ctx, src1, strlen(src1));
	printf("return value is: %lf\n", duk_get_number(ctx, -1));
	duk_pop(ctx);

	duk_eval_userbuffer(ctx, src2, strlen(src2));
	printf("result is: '%s'\n", duk_get_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_peval_userbuffer(ctx, src1, strlen(src1));
	printf("return value is: %s (rc=%d)\n", duk_safe_to_string(ctx, -1), rc);
	duk_pop(ctx);

	rc = duk_peval_userbuffer(ctx, src3, strlen(src3));
	printf("return value is: %s (rc=%d)\n", duk_safe_to_string(ctx, -1), rc);
	duk_pop(ctx);

	rc = duk_peval_userbuffer(ctx, src4, strlen(src4));
	printf("return value is: %s (rc=%d)\n", duk_safe_to_string(ctx, -1), rc);
	duk_pop(ctx);

	/* noresult variants */

	printf("top=%d\n", duk_get_top(ctx));
	duk_eval_userbuffer_noresult(ctx, src1, strlen(src1));

	printf("top=%d\n", duk_get_top(ctx));
	rc = duk_peval_userbuffer_noresult(ctx, src1, strlen(src1));
	printf("no result, rc=%d\n", rc);

	printf("top=%d\n", duk_get_top(ctx));
	rc = duk_peval_userbuffer_noresult(ctx, src5, strlen(src5));
	printf("no result, rc=%d\n", rc);

	printf("top: %d\n", duk_get_top(ctx));
}
