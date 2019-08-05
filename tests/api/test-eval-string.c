/*===
*** test_string (duk_safe_call)
Hello world!
return value is: 123.000000
result is: 'TESTSTRING'
Hello world!
return value is: 123 (rc=0)
return value is: Error: eval error (rc=1)
return value is: SyntaxError: invalid object literal (line 1, end of input) (rc=1)
top=0
Hello world!
top=0
Hello world!
no result, rc=0
top=0
no result, rc=1
top: 0
==> rc=0, result='undefined'
*** test_lstring (duk_safe_call)
Hello world!
return value is: 123.000000
result is: 'TESTSTRING'
Hello world!
return value is: 123 (rc=0)
return value is: Error: eval error (rc=1)
return value is: SyntaxError: invalid object literal (line 1, end of input) (rc=1)
top=0
Hello world!
top=0
Hello world!
no result, rc=0
top=0
no result, rc=1
top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_string(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	duk_eval_string(ctx, "print('Hello world!'); 123;");
	printf("return value is: %lf\n", duk_get_number(ctx, -1));
	duk_pop(ctx);

	duk_eval_string(ctx, "'testString'.toUpperCase()");
	printf("result is: '%s'\n", duk_get_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_peval_string(ctx, "print('Hello world!'); 123;");
	printf("return value is: %s (rc=%ld)\n", duk_safe_to_string(ctx, -1), (long) rc);
	duk_pop(ctx);

	rc = duk_peval_string(ctx, "throw new Error('eval error');");
	printf("return value is: %s (rc=%ld)\n", duk_safe_to_string(ctx, -1), (long) rc);
	duk_pop(ctx);

	rc = duk_peval_string(ctx, "throw new Error('eval error'); obj = {");
	printf("return value is: %s (rc=%ld)\n", duk_safe_to_string(ctx, -1), (long) rc);
	duk_pop(ctx);

	/* noresult variants */

	printf("top=%ld\n", (long) duk_get_top(ctx));
	duk_eval_string_noresult(ctx, "print('Hello world!'); 123;");

	printf("top=%ld\n", (long) duk_get_top(ctx));
	rc = duk_peval_string_noresult(ctx, "print('Hello world!'); 123;");
	printf("no result, rc=%ld\n", (long) rc);

	printf("top=%ld\n", (long) duk_get_top(ctx));
	rc = duk_peval_string_noresult(ctx, "print('Hello world!'); obj = {");
	printf("no result, rc=%ld\n", (long) rc);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_lstring(duk_context *ctx, void *udata) {
	duk_int_t rc;
	const char *src1 = "print('Hello world!'); 123;@";
	const char *src2 = "'testString'.toUpperCase()@";
	const char *src3 = "throw new Error('eval error');@";
	const char *src4 = "throw new Error('eval error'); obj = {@";
	const char *src5 = "print('Hello world!'); obj = {@";

	(void) udata;

	duk_eval_lstring(ctx, src1, strlen(src1) - 1);
	printf("return value is: %lf\n", duk_get_number(ctx, -1));
	duk_pop(ctx);

	duk_eval_lstring(ctx, src2, strlen(src2) - 1);
	printf("result is: '%s'\n", duk_get_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_peval_lstring(ctx, src1, strlen(src1) - 1);
	printf("return value is: %s (rc=%ld)\n", duk_safe_to_string(ctx, -1), (long) rc);
	duk_pop(ctx);

	rc = duk_peval_lstring(ctx, src3, strlen(src3) - 1);
	printf("return value is: %s (rc=%ld)\n", duk_safe_to_string(ctx, -1), (long) rc);
	duk_pop(ctx);

	rc = duk_peval_lstring(ctx, src4, strlen(src4) - 1);
	printf("return value is: %s (rc=%ld)\n", duk_safe_to_string(ctx, -1), (long) rc);
	duk_pop(ctx);

	/* noresult variants */

	printf("top=%ld\n", (long) duk_get_top(ctx));
	duk_eval_lstring_noresult(ctx, src1, strlen(src1) - 1);

	printf("top=%ld\n", (long) duk_get_top(ctx));
	rc = duk_peval_lstring_noresult(ctx, src1, strlen(src1) - 1);
	printf("no result, rc=%ld\n", (long) rc);

	printf("top=%ld\n", (long) duk_get_top(ctx));
	rc = duk_peval_lstring_noresult(ctx, src5, strlen(src5) - 1);
	printf("no result, rc=%ld\n", (long) rc);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_string);
	TEST_SAFE_CALL(test_lstring);
}
