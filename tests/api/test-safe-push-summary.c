/*===
duk_is_string()=1
'pig'
'the big fat pig ate everyone and...'
duk_is_string()=1
812
9000.1
false
true
null
undefined
Error: the pig ate it
===*/

void test(duk_context *ctx) {
	duk_push_string(ctx, "pig");
	duk_safe_push_summary(ctx, -1, 0x0);
	printf("duk_is_string()=%d\n", duk_is_string(ctx, -1));
	printf("%s\n", duk_get_string(ctx, -1));
	duk_pop_2(ctx);

	duk_push_string(ctx, "the big fat pig ate everyone and now it's too fat");
	duk_safe_push_summary(ctx, -1, 0x0);
	printf("%s\n", duk_get_string(ctx, -1));
	duk_pop_2(ctx);

	duk_push_int(ctx, 812);
	duk_safe_push_summary(ctx, -1, 0x0);
	printf("duk_is_string()=%d\n", duk_is_string(ctx, -1));
	printf("%s\n", duk_get_string(ctx, -1));
	duk_pop_2(ctx);

	duk_push_number(ctx, 9000.1);
	duk_safe_push_summary(ctx, -1, 0x0);
	printf("%s\n", duk_get_string(ctx, -1));
	duk_pop_2(ctx);

	duk_push_false(ctx);
	duk_safe_push_summary(ctx, -1, 0x0);
	printf("%s\n", duk_get_string(ctx, -1));
	duk_pop_2(ctx);

	duk_push_true(ctx);
	duk_safe_push_summary(ctx, -1, 0x0);
	printf("%s\n", duk_get_string(ctx, -1));
	duk_pop_2(ctx);

	duk_push_null(ctx);
	duk_safe_push_summary(ctx, -1, 0x0);
	printf("%s\n", duk_get_string(ctx, -1));
	duk_pop_2(ctx);

	duk_push_undefined(ctx);
	duk_safe_push_summary(ctx, -1, 0x0);
	printf("%s\n", duk_get_string(ctx, -1));
	duk_pop_2(ctx);

	duk_push_error(ctx, DUK_ERR_ERROR, "the pig ate it");
	duk_safe_push_summary(ctx, -1, 0x0);
	duk_safe_push_summary(ctx, -2, DUK_SUMMARY_ERROR_AWARE);
	printf("%s\n", duk_get_string(ctx, -2));
	//printf("%s\n", duk_get_string(ctx, -1));
	duk_pop_3(ctx);
}
