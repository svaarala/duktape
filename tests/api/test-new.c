/*===
*** test_new (duk_safe_call)
duk_is_constructor_call: 1
key='own_key', value='own_value'
key='name', value='my prototype'
duk_is_constructor_call: 1
key='name', value='replacement'
final top: 0
==> rc=0, result='undefined'
*** test_pnew_1 (duk_safe_call)
pnew returned: 0
result: Error: my error message
final top: 0
==> rc=0, result='undefined'
*** test_pnew_2 (duk_safe_call)
pnew returned: 1
result: TypeError: not constructable
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t my_func_1(duk_context *ctx) {
	printf("duk_is_constructor_call: %d\n", (int) duk_is_constructor_call(ctx));

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

static duk_ret_t my_func_2(duk_context *ctx) {
	printf("duk_is_constructor_call: %d\n", (int) duk_is_constructor_call(ctx));

	/* this replaces the freshly constructed object, and does NOT have
	 * the same internal prototype as the fresh object.
	 */
	duk_eval_string(ctx, "({ name: 'replacement' })");
	return 1;
}

static duk_ret_t test_new(duk_context *ctx) {
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

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Successful duk_pnew(). */
static duk_ret_t test_pnew_1(duk_context *ctx) {
	duk_int_t rc;

	duk_eval_string(ctx, "Error");  /* constructor */
	duk_push_string(ctx, "my error message");
	rc = duk_pnew(ctx, 1);
	printf("pnew returned: %ld\n", (long) rc);
	printf("result: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Failed duk_pnew(). */
static duk_ret_t test_pnew_2(duk_context *ctx) {
	duk_int_t rc;

	duk_eval_string(ctx, "null");  /* constructor: not callable */
	duk_push_string(ctx, "whatever");
	duk_push_string(ctx, "whatever");
	duk_push_string(ctx, "whatever");
	rc = duk_pnew(ctx, 3);
	printf("pnew returned: %ld\n", (long) rc);
	printf("result: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_new);
	TEST_SAFE_CALL(test_pnew_1);
	TEST_SAFE_CALL(test_pnew_2);
}
