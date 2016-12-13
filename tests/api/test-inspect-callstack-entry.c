/*
 *  duk_inspect_callstack_entry()
 */

/*===
*** test_basic (duk_safe_call)
result is undefined: 1
calling func
my_func called
inspect level -1
top before: 0
top after: 1
result is object: 1
result is undefined: 0
.function is object: 1
.function is undefined: 0
.function.name: my_func
inspect level -2
top before: 0
top after: 1
result is object: 1
result is undefined: 0
.function is object: 1
.function is undefined: 0
.function.name: myNamedFunction
inspect level -3
top before: 0
top after: 1
result is object: 0
result is undefined: 1
exited func
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t my_func(duk_context *ctx) {
	duk_int_t level;

	printf("my_func called\n");

	for (level = -1; level >= -100; level--) {
		printf("inspect level %ld\n", (long) level);
		printf("top before: %ld\n", (long) duk_get_top(ctx));
		duk_inspect_callstack_entry(ctx, level);
		printf("top after: %ld\n", (long) duk_get_top(ctx));
		printf("result is object: %ld\n", (long) duk_is_object(ctx, -1));
		printf("result is undefined: %ld\n", (long) duk_is_undefined(ctx, -1));
		if (duk_is_undefined(ctx, -1)) {
			break;
		}
		duk_get_prop_string(ctx, -1, "function");
		printf(".function is object: %ld\n", (long) duk_is_object(ctx, -1));
		printf(".function is undefined: %ld\n", (long) duk_is_undefined(ctx, -1));
		if (duk_is_object(ctx, -1)) {
			duk_get_prop_string(ctx, -1, "name");
			printf(".function.name: %s\n",  duk_safe_to_string(ctx, -1));
			duk_pop(ctx);
		}
		duk_pop_2(ctx);
	}

	return 0;
}

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_string(ctx, "dummy");

	/* Callstack is empty here, so undefined. */
	duk_inspect_callstack_entry(ctx, -1);
	printf("result is undefined: %ld\n", (long) duk_is_undefined(ctx, -1));
	duk_pop(ctx);

	/* Call a native target through eval code. */
	duk_eval_string(ctx,
		"(function myNamedFunction(func) {\n"
		"    print('calling func');\n"
		"    func();\n"
		"    print('exited func');\n"
		"})");
	duk_push_c_function(ctx, my_func, 0 /*nargs*/);
	duk_push_string(ctx, "name");
	duk_push_string(ctx, "my_func");
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE);
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_pop(ctx);  /* pop dummy */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
