/*===
Hello world!
return value is: 123.000000
adder(123, 234) -> 357.000000
Hello world!
return value is: 123 (rc=0)
return value is: Error: eval error (rc=1)
return value is: SyntaxError: invalid object literal (line 1) (rc=1)
top: 0
===*/

void test(duk_context *ctx) {
	int rc;

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
	printf("return value is: %s (rc=%d)\n", duk_safe_to_string(ctx, -1), rc);
	duk_pop(ctx);

	/* Protected eval with failure */
	duk_push_string(ctx, "throw new Error('eval error');");
	rc = duk_peval(ctx);
	printf("return value is: %s (rc=%d)\n", duk_safe_to_string(ctx, -1), rc);
	duk_pop(ctx);

	/* Protected eval with syntax error */
	duk_push_string(ctx, "obj = {");
	rc = duk_peval(ctx);
	printf("return value is: %s (rc=%d)\n", duk_safe_to_string(ctx, -1), rc);
	duk_pop(ctx);

	printf("top: %d\n", duk_get_top(ctx));
}
