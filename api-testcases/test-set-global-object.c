/*
 *  Set global object
 *
 *  The duk_set_global_object() API call replaces the global object with the
 *  specified object.  It also modifies the internal global lexical environment
 *  object so that it variable lookups are bound to the new global object.
 *
 *  This is a basic sandboxing primitive.  You can use this API call to build
 *  a subset global object with no access to dangerous primitives like eval(),
 *  the Duktape object, etc.  Because you are building a new object, you can
 *  set the configurability, enumerability, and writability flags of every value
 *  as you wish, even if the original value is write protected.
 */

/*===
*** test_invalid_index (duk_safe_call)
==> rc=1, result='TypeError: unexpected type'
*** test_invalid_target (duk_safe_call)
==> rc=1, result='TypeError: unexpected type'
*** test_basic (duk_safe_call)
build replacement global object
top before: 1
top after: 0
key: print
key: JSON
key: eval
key: newGlobal
key: testName
indirect eval
key: print
key: JSON
key: eval
key: newGlobal
key: testName
key: myEval
true
access through this.xxx and variable lookup xxx
this.testName: my new global
testName: my new global
final top: 0
==> rc=0, result='undefined'
*** test_noeval (duk_safe_call)
top before: 1
top after: 0
hello from C eval
result: 123
result: ReferenceError: identifier 'eval' undefined
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_invalid_index(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_set_global_object(ctx);
	return 0;
}

static duk_ret_t test_invalid_target(duk_context *ctx) {
	duk_push_int(ctx, 123);
	duk_set_global_object(ctx);
	return 0;
}

static void dump_global_object_keys(duk_context *ctx) {
	duk_eval_string_noresult(ctx,
		"(function () {\n"
		"    var k;\n"
		"    for (k in this) { print('key:', k); }\n"
		"})()\n");
}

static duk_ret_t test_basic(duk_context *ctx) {
	/*
	 *  First, build a new global object which contains a few keys we
	 *  want to see.
	 */

	printf("build replacement global object\n");
	duk_eval_string(ctx,
		"({\n"
		"    print: this.print,\n"
		"    JSON: this.JSON,\n"
		"    eval: this.eval,\n"
		"    newGlobal: true,\n"
		"    testName: 'my new global'\n"
		"})\n");

	printf("top before: %ld\n", (long) duk_get_top(ctx));
	duk_set_global_object(ctx);
	printf("top after: %ld\n", (long) duk_get_top(ctx));

	/*
	 *  Print available keys.  This exercises access to the global object
	 *  directly.
	 */
	dump_global_object_keys(ctx);

	/*
	 *  Test that indirect eval executes in the new global object too.
	 */
	printf("indirect eval\n");
	duk_eval_string_noresult(ctx,
		"var myEval = eval;  // writes 'myEval' to global object as a side effect\n"
	);
	dump_global_object_keys(ctx);
	duk_eval_string_noresult(ctx,
		"myEval('print(this.newGlobal)');"
	);

	/*
	 *  Test access to global object through an object environment.
	 */
	printf("access through this.xxx and variable lookup xxx\n");
	duk_eval_string_noresult(ctx,
		"print('this.testName:', this.testName);\n"
	);
	duk_eval_string_noresult(ctx,
		"print('testName:', testName);\n"
	);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_noeval(duk_context *ctx) {
	/*
	 *  Build a global environment with no eval - check that we can't
	 *  eval stuff anymore from Ecmascript code.  The C eval APIs still
	 *  work.
	 */

	duk_eval_string(ctx,
		"({\n"
		"    print: this.print\n"
		"})\n");

	printf("top before: %ld\n", (long) duk_get_top(ctx));
	duk_set_global_object(ctx);
	printf("top after: %ld\n", (long) duk_get_top(ctx));

	/*
	 *  Eval with C API.
	 */
	(void) duk_peval_string(ctx,
		"print('hello from C eval'); 123\n"
	);
	printf("result: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	/*
	 *  Eval with Ecmascript.
	 */
	(void) duk_peval_string(ctx,
		"eval('print(\"hello from Ecmascript eval\")')\n"
	);
	printf("result: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_invalid_index);
	TEST_SAFE_CALL(test_invalid_target);
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_noeval);
}
