/*===
still here
===*/

void test(duk_context *ctx) {
	duk_gc(ctx, 0);
	duk_gc(ctx, 0);
	duk_gc(ctx, 0);
	printf("still here\n");
}
