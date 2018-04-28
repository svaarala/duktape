/*===
Hello world from ECMAScript!
Hello world from C!
===*/

void test(duk_context *ctx) {
	duk_push_string(ctx, "print('Hello world from ECMAScript!');");
	duk_eval(ctx);
	printf("Hello world from C!\n");
}
