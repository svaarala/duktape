/*===
duk_is_object(1) = 1
.toString rc=0 -> undefined
json encoded: {"meaningOfLife":42}
top=2
===*/

void test(duk_context *ctx) {
	duk_idx_t obj_idx;
	duk_bool_t rc;

	duk_push_int(ctx, 123);  /* dummy */

	obj_idx = duk_push_bare_object(ctx);
	duk_push_int(ctx, 42);
	duk_put_prop_string(ctx, obj_idx, "meaningOfLife");

	printf("duk_is_object(%ld) = %d\n", (long) obj_idx, (int) duk_is_object(ctx, obj_idx));

	rc = duk_get_prop_string(ctx, obj_idx, "toString");
	printf(".toString rc=%ld -> %s\n", (long) rc, duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	duk_json_encode(ctx, obj_idx);  /* in-place */

	printf("json encoded: %s\n", duk_get_string(ctx, obj_idx));

	printf("top=%ld\n", (long) duk_get_top(ctx));
}
