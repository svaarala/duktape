/*===
{"meaningOfLife":42}
{"meaningOfLife":42,"foo":"bar"}
{"meaningOfLife":42,"foo":"bar"}
===*/

void test(duk_context *ctx) {
	duk_push_object(ctx);                           /* [ ... obj ] */
	duk_push_int(ctx, 42);                          /* [ ... obj 42 ] */
	duk_put_prop_string(ctx, -2, "meaningOfLife");  /* [ ... obj ] */

	/* compaction has no external impact */
	duk_compact(ctx, -1);                           /* [ ... obj ] */
	duk_dup_top(ctx);
	printf("%s\n", duk_json_encode(ctx, -1));
	duk_pop(ctx);

	/* compaction doesn't prevent new properties from being added */
	duk_push_string(ctx, "bar");                    /* [ ... obj "bar" ] */
	duk_put_prop_string(ctx, -2, "foo");            /* [ ... obj ] */
	duk_dup_top(ctx);
	printf("%s\n", duk_json_encode(ctx, -1));
	duk_pop(ctx);

	/* compaction can be done multiple times */
	duk_compact(ctx, -1);                           /* [ ... obj ] */
	duk_dup_top(ctx);
	printf("%s\n", duk_json_encode(ctx, -1));
	duk_pop(ctx);
}
