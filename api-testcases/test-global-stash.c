/*===
top: 0
top: 1
top: 0
value: 123
top: 0
===*/

void test(duk_context *ctx) {
	printf("top: %d\n", duk_get_top(ctx));
	duk_push_global_stash(ctx);
	printf("top: %d\n", duk_get_top(ctx));

	duk_push_int(ctx, 123);
	duk_put_prop_string(ctx, -2, "myvalue");
	duk_pop(ctx);
	printf("top: %d\n", duk_get_top(ctx));

	duk_push_global_stash(ctx);
	duk_get_prop_string(ctx, -1, "myvalue");
	printf("value: %d\n", duk_get_int(ctx, -1));
	duk_pop(ctx);
	duk_pop(ctx);
	printf("top: %d\n", duk_get_top(ctx));
}
