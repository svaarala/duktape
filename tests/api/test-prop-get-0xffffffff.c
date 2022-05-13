/*===
foo
bar
foo
bar
done
===*/

void test(duk_context *ctx) {
	duk_eval_string(ctx, "({ 4294967295: 'foo' })");
	duk_eval_string(ctx, "({ '4294967295': 'bar' })");

	(void) duk_get_prop_string(ctx, 0, "4294967295");
	printf("%s\n", duk_to_string(ctx, -1));
	(void) duk_get_prop_string(ctx, 1, "4294967295");
	printf("%s\n", duk_to_string(ctx, -1));
	(void) duk_get_prop_index(ctx, 0, 0xffffffffUL);
	printf("%s\n", duk_to_string(ctx, -1));
	(void) duk_get_prop_index(ctx, 1, 0xffffffffUL);
	printf("%s\n", duk_to_string(ctx, -1));

	printf("done\n");
}
