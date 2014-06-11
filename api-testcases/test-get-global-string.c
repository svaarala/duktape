/*===
top: 0
top: 1
ret: 1
encoded: foo%20bar
top: 0
top: 1
ret: 0
undefined
top: 0
===*/

void test(duk_context *ctx) {
	int ret;

	printf("top: %d\n", (int) duk_get_top(ctx));
	ret = duk_get_global_string(ctx, "encodeURIComponent");
	printf("top: %d\n", (int) duk_get_top(ctx));
	printf("ret: %d\n", ret);
	duk_push_string(ctx, "foo bar");
	duk_call(ctx, 1);
	printf("encoded: %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);
	printf("top: %d\n", (int) duk_get_top(ctx));

	ret = duk_get_global_string(ctx, "doesNotExist");
	printf("top: %d\n", (int) duk_get_top(ctx));
	printf("ret: %d\n", ret);
	printf("%s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);
	printf("top: %d\n", (int) duk_get_top(ctx));
}
