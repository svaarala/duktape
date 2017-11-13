/*
 *  duk_push_new_target()
 */

/*===
push with empty callstack, top 1, result type is 'undefined': 1
duk_is_constructor_call: 0
duk_is_undefined() for new.target: 1
duk_is_object() for new.target: 0
duk_is_callable() for new.target: 0
duk_get_c_function(): NULL
final top: 1
duk_is_constructor_call: 1
duk_is_undefined() for new.target: 0
duk_is_object() for new.target: 1
duk_is_callable() for new.target: 1
duk_get_c_function(): matches my_func: 1
final top: 1
final top: 0
===*/

static duk_ret_t my_func(duk_context *ctx) {
	duk_c_function func;

	printf("duk_is_constructor_call: %ld\n", (long) duk_is_constructor_call(ctx));
	duk_push_new_target(ctx);
	printf("duk_is_undefined() for new.target: %ld\n", duk_is_undefined(ctx, -1));
	printf("duk_is_object() for new.target: %ld\n", duk_is_object(ctx, -1));
	printf("duk_is_callable() for new.target: %ld\n", duk_is_callable(ctx, -1));

	func = duk_get_c_function(ctx, -1);
	if (func != NULL) {
		printf("duk_get_c_function(): matches my_func: %ld\n", (long) (func == my_func ? 1 : 0));
	} else {
		printf("duk_get_c_function(): NULL\n");
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	/* Push with empty callstack. */
	duk_push_new_target(ctx);
	printf("push with empty callstack, top %ld, result type is 'undefined': %ld\n",
	       (long) duk_get_top(ctx), (long) duk_is_undefined(ctx, -1));
	duk_pop(ctx);

	/* Push from normal call. */
	duk_push_c_function(ctx, my_func, 0);
	duk_call(ctx, 0);
	duk_pop(ctx);

	/* Push from constructor call. */
	duk_push_c_function(ctx, my_func, 0);
	duk_new(ctx, 0);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
}
