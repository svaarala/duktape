/*===
Hello world!
return value is: 123.000000
adder(123, 234) -> 357.000000
===*/

void test(duk_context *ctx) {
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
}
