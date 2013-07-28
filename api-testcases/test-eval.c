/*===
Hello world!
return value is: 123.000000
===*/

/* From API doc */

void test(duk_context *ctx) {
	duk_push_string(ctx, "print('Hello world!'); 123;");
	duk_eval(ctx);
	printf("return value is: %lf\n", duk_get_number(ctx, -1));
	duk_pop(ctx);
}

