/*
 *  Behavior of lightweight functions in various situations.
 *
 *  Also documents the detailed behavior and limitations of lightfuncs.
 */

/*===
FIXME
still here
===*/

/* FIXME: test all arg counts and lengths, including varargs */

/* FIXME: duk_to_object() coercion, stack policy */

/* FIXME: duk_push_current_function() for a lightfunc, check magic, perhaps length */

static duk_ret_t test_magic(duk_context *ctx) {
	/* FIXME */
	/* magic limits, C api test, check what happens if you exceed */
	return 0;
}

static duk_ret_t test_enum(duk_context *ctx) {
	/* FIXME: push lightfunc here instead of relying on a built-in */
	duk_eval_string(ctx, "Math.max");

	printf("enum defaults\n");
	duk_enum(ctx, -1, 0);
	while (duk_next(ctx, -1, 0 /*get_value*/)) {
		printf("key: %s\n", duk_to_string(ctx, -1));
		duk_pop(ctx);
	}
	duk_pop(ctx);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	printf("enum nonenumerable\n");
	duk_enum(ctx, -1, DUK_ENUM_INCLUDE_NONENUMERABLE);
	while (duk_next(ctx, -1, 0 /*get_value*/)) {
		printf("key: %s\n", duk_to_string(ctx, -1));
		duk_pop(ctx);
	}
	duk_pop(ctx);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	printf("enum own\n");
	duk_enum(ctx, -1, DUK_ENUM_OWN_PROPERTIES_ONLY);
	while (duk_next(ctx, -1, 0 /*get_value*/)) {
		printf("key: %s\n", duk_to_string(ctx, -1));
		duk_pop(ctx);
	}
	duk_pop(ctx);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	printf("enum own non-enumerable\n");
	duk_enum(ctx, -1, DUK_ENUM_OWN_PROPERTIES_ONLY | DUK_ENUM_INCLUDE_NONENUMERABLE);
	while (duk_next(ctx, -1, 0 /*get_value*/)) {
		printf("key: %s\n", duk_to_string(ctx, -1));
		duk_pop(ctx);
	}
	duk_pop(ctx);
	printf("top: %ld\n", (long) duk_get_top(ctx));

	return 0;
}

void test(duk_context *ctx) {
	/* nargs / length limits, C api test, check what happens if you exceed */
	/* Example of using lightfunc as a constructor, separate testcase, doc ref */

	TEST_SAFE_CALL(test_magic);
	TEST_SAFE_CALL(test_enum);

	printf("still here\n");
	fflush(stdout);
}
