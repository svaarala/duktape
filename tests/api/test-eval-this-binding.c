/*
 *  Test 'this' binding for eval code: https://github.com/svaarala/duktape/issues/164
 */

/*===
*** test_1 (duk_safe_call)
non-strict eval from C
not an error in non-strict mode
object
object
strict eval from C
ReferenceError
object
object
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	/* For non-strict eval code the 'this' binding was effectively
	 * the global object even before fixing GH-164: an undefined
	 * 'this' binding gets promoted to the global object by the
	 * usual Ecmascript call handling semantics.
	 */
	duk_eval_string(ctx,
		"print('non-strict eval from C');\n"
		"try { dummy1 = 1; print('not an error in non-strict mode'); } catch (e) { print(e.name); }\n"
		"print(typeof this);\n"
		"print(typeof (this || {}).Math);\n"
	);
	duk_pop(ctx);

	/* For strict eval code the 'this' binding was undefined when
	 * eval was being done from C code (but was the global object
	 * when done from Ecmascript code).  With GH-164 fixed the
	 * 'this' binding is global object for strict eval code too.
	 */

	duk_eval_string_noresult(ctx,
		"'use strict'\n"
		"print('strict eval from C');\n"
		"try { dummy2 = 1; print('error in strict mode'); } catch (e) { print(e.name); }\n"
		"print(typeof this);\n"
		"print(typeof (this || {}).Math);\n"
	);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
