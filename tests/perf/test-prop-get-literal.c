/*===
done
===*/

void test(duk_context *ctx) {
	unsigned long i;

	duk_eval_string(ctx, "({ foo: 123, bar: 234, quux: 345 })");

	for (i = 0; i < 1000000; i++) {
		duk_get_prop_literal(ctx, -1, "foo");
		duk_get_prop_literal(ctx, -2, "bar");
		duk_get_prop_literal(ctx, -3, "quux");
		duk_get_prop_literal(ctx, -4, "foo");
		duk_get_prop_literal(ctx, -5, "bar");
		duk_get_prop_literal(ctx, -6, "quux");
		duk_get_prop_literal(ctx, -7, "foo");
		duk_get_prop_literal(ctx, -8, "bar");
		duk_get_prop_literal(ctx, -9, "quux");
		duk_get_prop_literal(ctx, -10, "foo");
		duk_pop_n(ctx, 10);
	}

	printf("done\n");
}
