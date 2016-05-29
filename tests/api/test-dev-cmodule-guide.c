/*
 *  Module example from guide, as a more concrete test case.
 */

/* Include duktape.h and whatever platform headers are needed. */
#include "duktape.h"

/* No longer applicable because module framework is no longer built in. */
/*---
{
    "skip": true
}
---*/

static duk_ret_t my_print(duk_context *ctx) {
	duk_push_string(ctx, " ");
	duk_insert(ctx, 0);
	duk_join(ctx, duk_get_top(ctx) - 1);
	printf("%s\n", duk_safe_to_string(ctx, -1));
	return 0;
}

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

/*===
*** test_use_module (duk_safe_call)
my_func_2() called
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_use_module(duk_context *ignored_ctx, void *udata) {
	duk_context *ctx;

	(void) udata;

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

/*
 *  Example of how a modSearch() function can use module.exports to return
 *  the C module initialization value.
 */

/*===
*** test_modsearch_module (duk_safe_call)
1
my_func_1() called
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t my_modsearch(duk_context *ctx) {
	/* Arguments: id, require, exports, module.
	 *
	 * The 'id' is ignored in this example, normally you'd use 'id' to
	 * select which module to initialize.
	 */

	/* Initialize the C module. */
	duk_push_c_function(ctx, dukopen_my_module, 0);
	duk_call(ctx, 0);

	/* Result is now on stack top.  Overwrite module.exports to make
	 * that value come out from require().
	 */

	/* [ id require exports module c_module ] */
	duk_put_prop_string(ctx, 3 /*module*/, "exports");  /* module.exports = c_module; */

	return 0;  /* return undefined, no Ecmascript source code */
}

static duk_ret_t test_modsearch_module(duk_context *ignored_ctx, void *udata) {
	duk_context *ctx;

	(void) udata;

	ctx = duk_create_heap_default();
	if (!ctx) {
		printf("Failed to create heap\n");
		return 0;
	}

	/* Dummy print() binding. */
	duk_push_c_function(ctx, my_print, 1);
	duk_put_global_string(ctx, "print");

	/* Register Duktape.modSearch. */
	duk_eval_string(ctx, "(function (fun) { Duktape.modSearch = fun; })");
	duk_push_c_function(ctx, my_modsearch, 4 /*nargs*/);
	duk_call(ctx, 1);
	duk_pop(ctx);

	/* Require test. */
	duk_eval_string_noresult(ctx, "var mod = require('my_module'); print(mod.FLAG_FOO); mod.func1();");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	duk_destroy_heap(ctx);
	return 0;
}

/*
 *  Main
 */

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_use_module);
	TEST_SAFE_CALL(test_modsearch_module);
}
