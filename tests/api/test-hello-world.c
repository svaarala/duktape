/*===
Hello world from Ecmascript!
Hello world from C!
===*/

void test(duk_context *ctx) {
	duk_push_string(ctx, "print('Hello world from Ecmascript!');");
	duk_eval(ctx);
	printf("Hello world from C!\n");
}
