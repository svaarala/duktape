/*===
duk_is_constructor_call: 1
key='own_key', value='own_value'
key='name', value='my prototype'
duk_is_constructor_call: 1
key='name', value='replacement'
final top: 0
rc=0, ret=undefined
===*/

int my_func_1(duk_context *ctx) {
	printf("duk_is_constructor_call: %d\n", duk_is_constructor_call(ctx));

	/* check 'this'; it should have internal prototype set below to
	 * constructor's "prototype" property.
	 */
	duk_push_this(ctx);
	duk_push_string(ctx, "own_value");
	duk_put_prop_string(ctx, -2, "own_key");
	duk_enum(ctx, -1, 0 /*enum_flags*/);
	while (duk_next(ctx, -1, 1 /*get_value*/)) {
		printf("key='%s', value='%s'\n", duk_to_string(ctx, -2), duk_to_string(ctx, -1));
		duk_pop_2(ctx);
	}
	duk_pop(ctx);
	duk_pop(ctx);

	return 0;
}

int my_func_2(duk_context *ctx) {
	printf("duk_is_constructor_call: %d\n", duk_is_constructor_call(ctx));

	/* this replaces the freshly constructed object, and does NOT have
	 * the same internal prototype as the fresh object.
	 */
	duk_eval_string(ctx, "({ name: 'replacement' })");
	return 1;
}

int test_raw(duk_context *ctx) {
	/* Create a constructor (function object).  Its "prototype" property
	 * controls the internal prototype of created instances.
	 */

	duk_push_c_function(ctx, my_func_1, 0);
	duk_eval_string(ctx, "({ name: 'my prototype' })");
	duk_put_prop_string(ctx, -2, "prototype");
	duk_new(ctx, 0);   /* -> [ ret ] */
	duk_pop(ctx);

	/* If constructor returns an object, it replaces the fresh object
	 * created for the constructor "this" binding.
	 */

	duk_push_c_function(ctx, my_func_2, 0);
	duk_eval_string(ctx, "({ name: 'my prototype' })");
	duk_put_prop_string(ctx, -2, "prototype");
	duk_new(ctx, 0);   /* -> [ ret ] */
	duk_enum(ctx, -1, 0 /*enum_flags*/);
	while (duk_next(ctx, -1, 1 /*get_value*/)) {
		printf("key='%s', value='%s'\n", duk_to_string(ctx, -2), duk_to_string(ctx, -1));
		duk_pop_2(ctx);
	}
	duk_pop(ctx);
	duk_pop(ctx);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	int rc;

	rc = duk_safe_call(ctx, test_raw, 0, 1, DUK_INVALID_INDEX);
	printf("rc=%d, ret=%s\n", rc, duk_to_string(ctx, -1));
}

