/*===
program code
return value is: '123'
===*/

void test(duk_context *ctx) {
	duk_compile_string(ctx, 0, "print('program code'); 123");
	duk_call(ctx, 0);
	printf("return value is: '%s'\n", duk_to_string(ctx, -1));
	duk_pop(ctx);
}

