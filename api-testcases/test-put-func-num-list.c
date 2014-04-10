/*===
*** test_1 (duk_safe_call)
after definition, top=0
object
tweak,adjust,frobnicate,FLAG_FOO,FLAG_BAR,FLAG_QUUX,meaning
1
2
4
42
tweak, top=3
3
adjust, top=3
4
frobnicate, top=3
5
final top: 0
==> rc=0, result='undefined'
===*/

static int do_tweak(duk_context *ctx) {
	printf("tweak, top=%d\n", duk_get_top(ctx));
	duk_push_int(ctx, duk_get_int(ctx, 0) + duk_get_int(ctx, 1));
	return 1;
}

static int do_adjust(duk_context *ctx) {
	printf("adjust, top=%d\n", duk_get_top(ctx));
	duk_push_int(ctx, duk_get_int(ctx, 0) + duk_get_int(ctx, 2));
	return 1;
}

static int do_frobnicate(duk_context *ctx) {
	printf("frobnicate, top=%d\n", duk_get_top(ctx));
	duk_push_int(ctx, duk_get_int(ctx, 1) + duk_get_int(ctx, 2));
	return 1;
}

static const duk_functionlist_entry my_funcs[] = {
	{ "tweak", do_tweak },
	{ "adjust", do_adjust },
	{ "frobnicate", do_frobnicate },
	{ NULL, NULL }
};

static const duk_numberlist_entry my_consts[] = {
	{ "FLAG_FOO", (double) (1 << 0) },
	{ "FLAG_BAR", (double) (1 << 1) },
	{ "FLAG_QUUX", (double) (1 << 2) },
	{ "meaning", 42.0 },
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

	printf("after definition, top=%d\n", duk_get_top(ctx));

	/* Eval test. */
	duk_eval_string_noresult(ctx,
	    "print(typeof MyModule);\n"
	    "print(Object.getOwnPropertyNames(MyModule));\n"
	    "print(MyModule.FLAG_FOO);\n"
	    "print(MyModule.FLAG_BAR);\n"
	    "print(MyModule.FLAG_QUUX);\n"
	    "print(MyModule.meaning);\n"
	    "print(MyModule.tweak(1, 2, 3));\n"
	    "print(MyModule.adjust(1, 2, 3));\n"
	    "print(MyModule.frobnicate(1, 2, 3));\n"
	);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
