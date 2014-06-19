/*===
top: 2
top: 3
top: 2
top: 3
top: 2
value: 123
top: 2
value: 234
top: 2
===*/

void test(duk_context *ctx) {
	duk_context *ctx1;
	duk_context *ctx2;

	duk_push_thread(ctx);
	ctx1 = duk_get_context(ctx, -1);
	duk_push_thread(ctx);
	ctx2 = duk_get_context(ctx, -1);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	duk_push_thread_stash(ctx, ctx1);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	duk_push_int(ctx, 123);
	duk_put_prop_string(ctx, -2, "myvalue");
	duk_pop(ctx);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	duk_push_thread_stash(ctx, ctx2);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	duk_push_int(ctx, 234);
	duk_put_prop_string(ctx, -2, "myvalue");
	duk_pop(ctx);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	duk_push_thread_stash(ctx, ctx1);
	duk_get_prop_string(ctx, -1, "myvalue");
	printf("value: %ld\n", (long) duk_get_int(ctx, -1));
	duk_pop(ctx);
	duk_pop(ctx);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	duk_push_thread_stash(ctx, ctx2);
	duk_get_prop_string(ctx, -1, "myvalue");
	printf("value: %ld\n", (long) duk_get_int(ctx, -1));
	duk_pop(ctx);
	duk_pop(ctx);
	printf("top: %ld\n", (long) duk_get_top(ctx));
}
