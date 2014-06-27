/*===
*** test_1 (duk_safe_call)
after definition, top=0
object
tweak,adjust,frobnicate,FLAG_FOO,FLAG_BAR,FLAG_QUUX,meaning
1
2
4
42
tweak, top=0
0
adjust, top=3
4
frobnicate, top=6
5
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t do_tweak(duk_context *ctx) {
	/* nargs=0 */
	printf("tweak, top=%ld\n", (long) duk_get_top(ctx));
	duk_push_int(ctx, duk_get_int(ctx, 0) + duk_get_int(ctx, 1));
	return 1;
}

static duk_ret_t do_adjust(duk_context *ctx) {
	/* nargs=3 */
	printf("adjust, top=%ld\n", (long) duk_get_top(ctx));
	duk_push_int(ctx, duk_get_int(ctx, 0) + duk_get_int(ctx, 2));
	return 1;
}

static duk_ret_t do_frobnicate(duk_context *ctx) {
	/* nargs=VARARGS */
	printf("frobnicate, top=%ld\n", (long) duk_get_top(ctx));
	duk_push_int(ctx, duk_get_int(ctx, 1) + duk_get_int(ctx, 2));
	return 1;
}

static const duk_function_list_entry my_funcs[] = {
	{ "tweak", do_tweak, 0 },
	{ "adjust", do_adjust, 3 },
	{ "frobnicate", do_frobnicate, DUK_VARARGS },
	{ NULL, NULL, 0 }
};

static const duk_number_list_entry my_consts[] = {
	{ "FLAG_FOO", (duk_double_t) (1 << 0) },
	{ "FLAG_BAR", (duk_double_t) (1 << 1) },
	{ "FLAG_QUUX", (duk_double_t) (1 << 2) },
	{ "meaning", (duk_double_t) 42.0 },
	{ NULL, 0.0 }
};

int test_1(duk_context *ctx) {
	/* This becomes our module. */
	duk_push_object(ctx);
	duk_push_string(ctx, "dummy");  /* just for offset */

	/* Define functions and constants. */
	duk_put_function_list(ctx, -2, my_funcs);
	duk_put_number_list(ctx, -2, my_consts);

	/* Define module into global object. */
	duk_push_global_object(ctx);
	duk_dup(ctx, -3);
	duk_put_prop_string(ctx, -2, "MyModule");
	duk_pop_3(ctx);  /* global, dummy, object */

	printf("after definition, top=%ld\n", (long) duk_get_top(ctx));

	/* Eval test. */
	duk_eval_string_noresult(ctx,
	    "print(typeof MyModule);\n"
	    "print(Object.getOwnPropertyNames(MyModule));\n"
	    "print(MyModule.FLAG_FOO);\n"
	    "print(MyModule.FLAG_BAR);\n"
	    "print(MyModule.FLAG_QUUX);\n"
	    "print(MyModule.meaning);\n"
	    "print(MyModule.tweak(1, 2, 3, 4, 5, 6));\n"
	    "print(MyModule.adjust(1, 2, 3, 4, 5, 6));\n"
	    "print(MyModule.frobnicate(1, 2, 3, 4, 5, 6));\n"
	);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
