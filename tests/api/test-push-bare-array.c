/*===
arr_idx = 1
prototype is undefined: 1
duk_is_array(1) = 1
arr[5] = 'undefined'
json encoded: ["foo","bar"]
top=2
===*/

void test(duk_context *ctx) {
	duk_idx_t arr_idx;

	duk_push_int(ctx, 123);  /* dummy */

	arr_idx = duk_push_bare_array(ctx);
	printf("arr_idx = %ld\n", (long) arr_idx);
	duk_get_prototype(ctx, -1);
	printf("prototype is undefined: %ld\n", (long) duk_is_undefined(ctx, -1));
	duk_pop(ctx);

	duk_push_string(ctx, "foo");
	duk_put_prop_index(ctx, arr_idx, 0);
	duk_push_string(ctx, "bar");
	duk_put_prop_index(ctx, arr_idx, 1);

	/* Array is now: [ "foo", "bar" ], and array.length is 2 (automatically
	 * updated for ECMAScript arrays).  The array being bare does not affect
	 * JSON serialization.
	 */

	printf("duk_is_array(%ld) = %d\n", (long) arr_idx, (int) duk_is_array(ctx, arr_idx));

	duk_eval_string_noresult(ctx, "Array.prototype[5] = 'inherit';");
	(void) duk_get_prop_index(ctx, arr_idx, 5);
	printf("arr[5] = '%s'\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	duk_json_encode(ctx, arr_idx);  /* in-place */
	printf("json encoded: %s\n", duk_get_string(ctx, arr_idx));

	printf("top=%ld\n", (long) duk_get_top(ctx));
}
