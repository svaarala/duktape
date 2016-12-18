/*
 *  Symbol related tests which can only be done from the C API.
 */

/*===
*** test_push_string (duk_safe_call)
symbol
Symbol(mySymbol)
final top: 0
==> rc=0, result='undefined'
*** test_traceback (duk_safe_call)
Error: aiee
    at [anon] (:1)
    at test (eval:3) preventsyield
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_push_string(duk_context *ctx, void *udata) {
	(void) udata;

	/* duk_push_(l)string() always pushes a string, but that string gets
	 * interpreted as a symbol if it is invalid CESU-8 / extended UTF-8
	 * and matches the symbol internal string formats.
	 */
	duk_eval_string(ctx, "(function (v) { print(typeof v); print(String(v)); })");
	duk_push_string(ctx, "\x81" "mySymbol" "\xff" "unique1234");
	duk_call(ctx, 1);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_traceback(duk_context *ctx, void *udata) {
	duk_eval_string(ctx,
		"(function test(fn) {\n"
		"    try {\n"
		"        fn();\n"
		"    } catch (e) {\n"
		"        print(e.stack);\n"
		"    }\n"
		"})\n");

	/* If function .name and/or .fileName is a symbol, it is ignored
	 * in tracebacks.  This is very much a corner case, and can only
	 * be created from C code now to override the non-configurability
	 * of the properties.
	 */
	duk_eval_string(ctx, "(function foo() { throw new Error('aiee'); })");
	duk_push_string(ctx, "name");
	duk_push_string(ctx, "\x80" "nameSymbol");
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_FORCE);
	duk_push_string(ctx, "fileName");
	duk_push_string(ctx, "\x80" "fileNameSymbol");
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_FORCE);

	duk_call(ctx, 1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_push_string);
	TEST_SAFE_CALL(test_traceback);
}
