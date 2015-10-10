/*
 *  Fastint compatible integer constants should read back as fastints.
 */

/*---
{
    "specialoptions": "requires DUK_OPT_FASTINT"
}
---*/

/*===
*** test_1 (duk_safe_call)
4886718345
true
4886718345
true
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	/* copied from polyfills/duktape-isfastint.js */
	duk_eval_string_noresult(ctx,
		"Object.defineProperty(Duktape, 'fastintTag', {\n"
		"    /* Tag number depends on duk_tval packing. */\n"
		"    value: (Duktape.info(true)[1] >= 0xfff0) ?\n"
		"            0xfff1 /* tag for packed duk_tval */ :\n"
		"            1 /* tag for unpacked duk_tval */,\n"
		"    writable: false,\n"
		"    enumerable: false,\n"
		"    configurable: true\n"
		"});\n"
		"Object.defineProperty(Duktape, 'isFastint', {\n"
		"    value: function (v) {\n"
		"        return Duktape.info(v)[0] === 4 &&                 /* public type is DUK_TYPE_NUMBER */\n"
		"               Duktape.info(v)[1] === Duktape.fastintTag;  /* internal tag is fastint */\n"
		"    },\n"
                "    writable: false, enumerable: false, configurable: true\n"
		"});\n");

	/* Constant must be large enough to avoid a LDINT instead of
	 * LDCONST.
	 */
	duk_eval_string(ctx,
		"(function () {\n"
		"    var v1 = 0x123456789;\n"
		"    print(v1);\n"
		"    print(Duktape.isFastint(v1));\n"
		"})");

	duk_dup_top(ctx);
	duk_call(ctx, 0);
	duk_pop(ctx);

	duk_dump_function(ctx);
	duk_load_function(ctx);
	duk_call(ctx, 0);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
