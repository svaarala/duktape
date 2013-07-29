/*===
Hello world!
return value is: 123.000000
result is: 'TESTSTRING'
===*/

void test(duk_context *ctx) {
	duk_eval_string(ctx, "print('Hello world!'); 123;");
	printf("return value is: %lf\n", duk_get_number(ctx, -1));
	duk_pop(ctx);

	duk_eval_string(ctx, "'testString'.toUpperCase()");
	printf("result is: '%s'\n", duk_get_string(ctx, -1));
	duk_pop(ctx);
}

