/*===
Hello world!
return value is: 123.000000
adder(123, 234) -> 357.000000
Hello world!
return value is: 123 (rc=0)
return value is: Error: eval error (rc=1)
return value is: SyntaxError: invalid object literal (line 1, end of input) (rc=1)
top=0
doing eval
top=0
doing peval
rc=0
top=0
rc=1
top: 0
===*/

void test(duk_context *ctx) {
	duk_int_t rc;

	/* From API doc */
	duk_push_string(ctx, "print('Hello world!'); 123;");
	duk_eval(ctx);
	printf("return value is: %lf\n", duk_get_number(ctx, -1));
	duk_pop(ctx);

	/* Function expression */
	duk_push_string(ctx, "(function adder(x,y) { return x+y; })");
	duk_eval(ctx);  /* [ func ] */
	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	duk_call(ctx, 2);  /* [ func 123 234 ] -> [ result ] */
	printf("adder(123, 234) -> %lf\n", duk_get_number(ctx, -1));
	duk_pop(ctx);

	/* Protected eval with success */
	duk_push_string(ctx, "print('Hello world!'); 123;");
	rc = duk_peval(ctx);
	printf("return value is: %s (rc=%d)\n", duk_safe_to_string(ctx, -1), (int) rc);
	duk_pop(ctx);

	/* Protected eval with failure */
	duk_push_string(ctx, "throw new Error('eval error');");
	rc = duk_peval(ctx);
	printf("return value is: %s (rc=%d)\n", duk_safe_to_string(ctx, -1), (int) rc);
	duk_pop(ctx);

	/* Protected eval with syntax error */
	duk_push_string(ctx, "obj = {");
	rc = duk_peval(ctx);
	printf("return value is: %s (rc=%d)\n", duk_safe_to_string(ctx, -1), (int) rc);
	duk_pop(ctx);

	/* Plain eval with no result */
	printf("top=%ld\n", (long) duk_get_top(ctx));
	duk_push_string(ctx, "print('doing eval');");
	duk_eval_noresult(ctx);

	/* Protected eval with no result, no error */
	printf("top=%ld\n", (long) duk_get_top(ctx));
	duk_push_string(ctx, "print('doing peval');");
	rc = duk_peval_noresult(ctx);
	printf("rc=%d\n", (int) rc);

	/* Protected eval with no result, syntax error */
	printf("top=%ld\n", (long) duk_get_top(ctx));
	duk_push_string(ctx, "print('doing peval'); obj = {");
	rc = duk_peval_noresult(ctx);
	printf("rc=%d\n", (int) rc);

	printf("top: %ld\n", (long) duk_get_top(ctx));
}
