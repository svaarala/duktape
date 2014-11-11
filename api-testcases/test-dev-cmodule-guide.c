/*
 *  Module example from guide, as a more concrete test case.
 */

/*===
*** test_use_module (duk_safe_call)
my_func_2() called
final top: 0
==> rc=0, result='undefined'
===*/

/* Include duktape.h and whatever platform headers are needed. */
#include "duktape.h"

/*
 *  Duktape/C functions providing module functionality.
 */

static duk_ret_t my_func_1(duk_context *ctx) {
	printf("my_func_1() called\n");
	return 0;
}

static duk_ret_t my_func_2(duk_context *ctx) {
	printf("my_func_2() called\n");
	return 0;
}

/*
 *  Module initialization
 */

static const duk_function_list_entry my_module_funcs[] = {
	{ "func1", my_func_1, 3 /*nargs*/ },
	{ "func2", my_func_2, DUK_VARARGS /*nargs*/ },
	{ NULL, NULL, 0 }
};

static const duk_number_list_entry my_module_consts[] = {
	{ "FLAG_FOO", (double) (1 << 0) },
	{ NULL, 0.0 }
};

static duk_ret_t dukopen_my_module(duk_context *ctx) {
	duk_push_object(ctx);
	duk_put_function_list(ctx, -1, my_module_funcs);
	duk_put_number_list(ctx, -1, my_module_consts);
	return 1;
}

/*
 *  Calling code
 */

static duk_ret_t test_use_module(duk_context *ignored_ctx) {
	duk_context *ctx;

	ctx = duk_create_heap_default();
	if (!ctx) {
		printf("Failed to create heap\n");
		return 0;
	}

	duk_push_c_function(ctx, dukopen_my_module, 0);
	duk_call(ctx, 0);
	duk_put_global_string(ctx, "my_module");

	duk_eval_string_noresult(ctx, "my_module.func2()");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	duk_destroy_heap(ctx);
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_use_module);
}
