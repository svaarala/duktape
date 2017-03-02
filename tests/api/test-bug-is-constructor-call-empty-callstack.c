/*===
still here
===*/

void test(duk_context *ctx) {
	(void) duk_is_constructor_call(ctx);
	printf("still here\n");
}
