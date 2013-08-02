/*===
err_idx: 2
name: TypeError
message: invalid argument: 234
code: 105
final top: 3
===*/

void test(duk_context *ctx) {
	int err_idx;

	/* dummy values */
	duk_push_int(ctx, 123);
	duk_push_int(ctx, 123);

	err_idx = duk_push_error_object(ctx, DUK_ERR_TYPE_ERROR, "invalid argument: %d", 234);
	printf("err_idx: %d\n", err_idx);

	duk_get_prop_string(ctx, -1, "name");
	printf("name: %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "message");
	printf("message: %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "code");
	printf("code: %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %d\n", duk_get_top(ctx));
}
