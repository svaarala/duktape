/*===
duk_is_array(1) = 1
json encoded: ["foo","bar"]
top=2
===*/

void test(duk_context *ctx) {
	duk_idx_t arr_idx;

	duk_push_int(ctx, 123);  /* dummy */

	arr_idx = duk_push_array(ctx);
	duk_push_string(ctx, "foo");
	duk_put_prop_index(ctx, arr_idx, 0);
	duk_push_string(ctx, "bar");
	duk_put_prop_index(ctx, arr_idx, 1);

	/* array is now: [ "foo", "bar" ], and array.length is 2 (automatically
	 * updated for Ecmascript arrays).
	 */

	printf("duk_is_array(%ld) = %d\n", (long) arr_idx, (int) duk_is_array(ctx, arr_idx));

	duk_json_encode(ctx, arr_idx);  /* in-place */

	printf("json encoded: %s\n", duk_get_string(ctx, arr_idx));

	printf("top=%ld\n", (long) duk_get_top(ctx));
}
