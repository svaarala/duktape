/*===
type=6
ToString=[object global]
duk_is_function(global.escape)=1
===*/

void test(duk_context *ctx) {
	duk_push_global_object(ctx);
	printf("type=%d\n", (int) duk_get_type(ctx, -1));
	printf("ToString=%s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	duk_push_global_object(ctx);
	duk_get_prop_string(ctx, -1, "escape");
	printf("duk_is_function(global.escape)=%d\n", (int) duk_is_function(ctx, -1));
	duk_pop(ctx);
}
