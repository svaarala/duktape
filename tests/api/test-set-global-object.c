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
 *  set the configurability, enumerability, and writability flags of every
 *  value as you wish, even if the original value is write protected.
 *
 *  NOTE: don't call duk_set_global_object() for the initial 'ctx' given to
 *  test(), as it makes the tests order dependent.
 */

/*===
*** test_invalid_index (duk_safe_call)
==> rc=1, result='TypeError: object required, found none (stack index -1)'
*** test_invalid_target (duk_safe_call)
==> rc=1, result='TypeError: object required, found 123 (stack index -1)'
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
key: print
hello from C eval
result: 123
result: ReferenceError: identifier 'eval' undefined
final top: 0
==> rc=0, result='undefined'
*** test_regexp_literals (duk_safe_call)
result: ReferenceError: identifier 'RegExp' undefined
/foo/
function
result: undefined
key: print
key: re
final top: 0
==> rc=0, result='undefined'
*** test_regexp_prototype_shared (duk_safe_call)
ctx1
result: ReferenceError: identifier 'RegExp' undefined
ctx2
result: ReferenceError: identifier 'RegExp' undefined
ctx1
object
set proto foo to quux
set proto bar to proto itself for comparison
result: /(?:)/
ctx2
object
foo: quux
bar equals getProto(re2): true
result: undefined
globals of ctx1 at end
key: name
key: print
key: getProto
key: re1
globals of ctx2 at end
key: name
key: print
key: getProto
key: re2
final top ctx1: 0
final top ctx2: 0
==> rc=0, result='undefined'
*** test_set_after_thread_create (duk_safe_call)
global object keys for ctx1 before change
global object keys for ctx2 before change
replace global object for ctx1
global object keys for ctx1 after change
key: newScope
key: print
global object keys for ctx2 after change
Duktape lookup through 'this' and directly
undefined
result: undefined
result: ReferenceError: identifier 'Duktape' undefined
[object Object]
result: undefined
[object Object]
result: undefined
newScope lookup through 'this' and directly
my new scope
result: undefined
my new scope
result: undefined
undefined
result: undefined
result: ReferenceError: identifier 'newScope' undefined
final top ctx1: 0
final top ctx2: 0
==> rc=0, result='undefined'
*** test_set_before_thread_create (duk_safe_call)
global object keys for ctx1 before change
replace global object for ctx1
global object keys for ctx1 after change
key: newScope1
key: print
create ctx2 from ctx1, with copied globals
global object keys for ctx2 after creation
key: newScope1
key: print
replace global object for ctx2
global object keys for ctx1
key: newScope1
key: print
global object keys for ctx2
key: newScope2
key: print
create ctx3 from ctx1, with fresh globals
global object keys for ctx1
key: newScope1
key: print
global object keys for ctx2
key: newScope2
key: print
global object keys for ctx3
final top ctx1: 2
final top ctx2: 0
final top ctx3: 0
==> rc=0, result='undefined'
===*/

static void dump_global_object_keys(duk_context *ctx) {
	/* Prints only non-enumerable keys.  We can't use e.g.
	 * Object.getOwnPropertyNames() here because we might
	 * not have 'Object' any more.
	 */
	duk_eval_string_noresult(ctx,
		"(function () {\n"
		"    for (var k in this) { print('key:', k); }\n"
		"})()\n");
}

static duk_ret_t test_invalid_index(duk_context *ctx_root) {
	duk_context *ctx;

	duk_push_thread(ctx_root);
	ctx = duk_require_context(ctx_root, -1);

	duk_set_top(ctx, 0);
	duk_set_global_object(ctx);
	return 0;
}

static duk_ret_t test_invalid_target(duk_context *ctx_root) {
	duk_context *ctx;

	duk_push_thread(ctx_root);
	ctx = duk_require_context(ctx_root, -1);

	duk_push_int(ctx, 123);
	duk_set_global_object(ctx);
	return 0;
}

static duk_ret_t test_basic(duk_context *ctx_root) {
	duk_context *ctx;

	duk_push_thread(ctx_root);
	ctx = duk_require_context(ctx_root, -1);

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

static duk_ret_t test_noeval(duk_context *ctx_root) {
	duk_context *ctx;

	duk_push_thread(ctx_root);
	ctx = duk_require_context(ctx_root, -1);

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
	dump_global_object_keys(ctx);

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

static duk_ret_t test_regexp_literals(duk_context *ctx_root) {
	duk_context *ctx;

	duk_push_thread(ctx_root);
	ctx = duk_require_context(ctx_root, -1);

	/*
	 *  Despite having no RegExp constructor, the built-in RegExp
	 *  constructor can be accessed through regexp instances
	 *  created through regexp literals.
	 */

	duk_eval_string(ctx,
		"({\n"
		"    print: this.print\n"
		"})\n");
	duk_set_global_object(ctx);

	(void) duk_peval_string(ctx, "print('RegExp:', RegExp)");
	printf("result: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	(void) duk_peval_string(ctx,
		"var re = /foo/;\n"
		"print(re);\n"
		"print(typeof re.exec)\n");
	printf("result: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	dump_global_object_keys(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_regexp_prototype_shared(duk_context *ctx_root) {
	duk_context *ctx1;
	duk_context *ctx2;

	/*
	 *  The RegExp constructor (built-in) is shared between two
	 *  threads which have been created with the same global
	 *  environment, even if the global object is replaced after
	 *  thread creation.
	 *
	 *  The RegExp instance will be the same for both.  To avoid
	 *  this, use duk_push_thread_new_globalenv().
	 */

	duk_push_thread(ctx_root);
	ctx1 = duk_require_context(ctx_root, -1);
	duk_push_thread(ctx_root);
	ctx2 = duk_require_context(ctx_root, -1);

	duk_eval_string(ctx1,
		"({\n"
		"    name: 'ctx1',\n"
		"    print: this.print,\n"
		"    getProto: Object.getPrototypeOf\n"
		"})");
	duk_set_global_object(ctx1);

	duk_eval_string(ctx2,
		"({\n"
		"    name: 'ctx2',\n"
		"    print: this.print,\n"
		"    getProto: Object.getPrototypeOf\n"
		"})");
	duk_set_global_object(ctx2);

	/* no direct access to RegExp */

	(void) duk_peval_string(ctx1, "print(name); print('RegExp:', RegExp)");
	printf("result: %s\n", duk_safe_to_string(ctx1, -1));
	duk_pop(ctx1);

	(void) duk_peval_string(ctx2, "print(name); print('RegExp:', RegExp)");
	printf("result: %s\n", duk_safe_to_string(ctx2, -1));
	duk_pop(ctx2);

	/* access shared RegExp.prototype through a regexp instance */

	duk_eval_string_noresult(ctx1, "var re1 = /foo/;\n");
	duk_eval_string_noresult(ctx2, "var re2 = /bar/;\n");

	(void) duk_peval_string(ctx1,
		"print(name);\n"
		"print(typeof getProto(re1));\n"
		"print('set proto foo to quux');\n"
		"getProto(re1).foo = 'quux';\n"
		"print('set proto bar to proto itself for comparison');\n"
		"getProto(re1).bar = getProto(re1);\n");
	printf("result: %s\n", duk_safe_to_string(ctx1, -1));
	duk_pop(ctx1);

	(void) duk_peval_string(ctx2,
		"print(name);\n"
		"print(typeof getProto(re2));\n"
		"print('foo:', getProto(re2).foo);\n"
		"print('bar equals getProto(re2):', getProto(re2).bar === getProto(re2));\n");
	printf("result: %s\n", duk_safe_to_string(ctx2, -1));
	duk_pop(ctx2);

	/* dump global objects at the end */

	printf("globals of ctx1 at end\n");
	dump_global_object_keys(ctx1);
	printf("globals of ctx2 at end\n");
	dump_global_object_keys(ctx2);

	printf("final top ctx1: %ld\n", (long) duk_get_top(ctx1));
	printf("final top ctx2: %ld\n", (long) duk_get_top(ctx2));
	return 0;
}

static duk_ret_t test_set_after_thread_create(duk_context *ctx_root) {
	duk_context *ctx1;
	duk_context *ctx2;

	duk_push_thread(ctx_root);
	ctx1 = duk_require_context(ctx_root, -1);
	duk_push_thread(ctx_root);
	ctx2 = duk_require_context(ctx_root, -1);

	/*
	 *  Setting the global object after creating a new thread using
	 *  duk_push_thread() has no effect on the other thread which
	 *  originally shared the global environment.
	 *
	 *  Access the global object directly ("this.foo") and through
	 *  the global scope ("foo") to ensure both are updated properly
	 *  and that there's no "cross talk".
	 *
	 *  The built-in objects (like RegExp, Number, Duktape, etc) are
	 *  still shared by the threads if they are accessible.  To avoid
	 *  this, use duk_push_thread_new_globalenv().
	 */

	printf("global object keys for ctx1 before change\n");
	dump_global_object_keys(ctx1);
	printf("global object keys for ctx2 before change\n");
	dump_global_object_keys(ctx2);

	printf("replace global object for ctx1\n");
	duk_eval_string(ctx1,
		"({\n"
		"    newScope: 'my new scope',\n"
		"    print: this.print\n"
		"})");
	duk_set_global_object(ctx1);

	printf("global object keys for ctx1 after change\n");
	dump_global_object_keys(ctx1);
	printf("global object keys for ctx2 after change\n");
	dump_global_object_keys(ctx2);

	printf("Duktape lookup through 'this' and directly\n");
	(void) duk_peval_string(ctx1, "print(this.Duktape);\n");
	printf("result: %s\n", duk_safe_to_string(ctx1, -1));
	duk_pop(ctx1);
	(void) duk_peval_string(ctx1, "print(Duktape);\n");
	printf("result: %s\n", duk_safe_to_string(ctx1, -1));
	duk_pop(ctx1);
	(void) duk_peval_string(ctx2, "print(this.Duktape);\n");
	printf("result: %s\n", duk_safe_to_string(ctx2, -1));
	duk_pop(ctx2);
	(void) duk_peval_string(ctx2, "print(Duktape);\n");
	printf("result: %s\n", duk_safe_to_string(ctx2, -1));
	duk_pop(ctx2);

	printf("newScope lookup through 'this' and directly\n");
	(void) duk_peval_string(ctx1, "print(this.newScope);\n");
	printf("result: %s\n", duk_safe_to_string(ctx1, -1));
	duk_pop(ctx1);
	(void) duk_peval_string(ctx1, "print(newScope);\n");
	printf("result: %s\n", duk_safe_to_string(ctx1, -1));
	duk_pop(ctx1);
	(void) duk_peval_string(ctx2, "print(this.newScope);\n");
	printf("result: %s\n", duk_safe_to_string(ctx2, -1));
	duk_pop(ctx2);
	(void) duk_peval_string(ctx2, "print(newScope);\n");
	printf("result: %s\n", duk_safe_to_string(ctx2, -1));
	duk_pop(ctx2);

	printf("final top ctx1: %ld\n", (long) duk_get_top(ctx1));
	printf("final top ctx2: %ld\n", (long) duk_get_top(ctx2));
	return 0;
}

static duk_ret_t test_set_before_thread_create(duk_context *ctx_root) {
	duk_context *ctx1;
	duk_context *ctx2;
	duk_context *ctx3;

	/*
	 *  Creating a new thread using duk_push_thread() after setting
	 *  globals on the current thread causes the new thread to inherit
	 *  the new global object.
	 */

	duk_push_thread(ctx_root);
	ctx1 = duk_require_context(ctx_root, -1);
	printf("global object keys for ctx1 before change\n");
	dump_global_object_keys(ctx1);

	printf("replace global object for ctx1\n");
	duk_eval_string(ctx1,
		"({\n"
		"    newScope1: 'my new scope 1',\n"
		"    print: this.print\n"
		"})");
	duk_set_global_object(ctx1);

	printf("global object keys for ctx1 after change\n");
	dump_global_object_keys(ctx1);

	/* NOTE: here it is critical that duk_push_thread() is called for ctx1,
	 * not ctx_root!
	 */
	printf("create ctx2 from ctx1, with copied globals\n");
	duk_push_thread(ctx1);
	ctx2 = duk_require_context(ctx1, -1);

	/* Here ctx2 will have the replaced global object of ctx1. */
	printf("global object keys for ctx2 after creation\n");
	dump_global_object_keys(ctx2);

	/*
	 *  You can set another global object for the new thread; the two
	 *  thread are not linked in any way.  Inheritance of the global
	 *  object happens only during thread creation.
	 */

	printf("replace global object for ctx2\n");
	duk_eval_string(ctx2,
		"({\n"
		"    newScope2: 'my new scope 2',\n"
		"    print: this.print\n"
		"})");
	duk_set_global_object(ctx2);

	printf("global object keys for ctx1\n");
	dump_global_object_keys(ctx1);
	printf("global object keys for ctx2\n");
	dump_global_object_keys(ctx2);

	/*
	 *  However, if you create a thread with duk_push_thread_new_globalenv()
	 *  it gets fresh globals regardless of the previous context.
	 */

	printf("create ctx3 from ctx1, with fresh globals\n");

	/* NOTE: again, push on ctx1, not ctx_root. */
	duk_push_thread_new_globalenv(ctx1);
	ctx3 = duk_require_context(ctx1, -1);

	printf("global object keys for ctx1\n");
	dump_global_object_keys(ctx1);
	printf("global object keys for ctx2\n");
	dump_global_object_keys(ctx2);
	printf("global object keys for ctx3\n");
	dump_global_object_keys(ctx3);

	printf("final top ctx1: %ld\n", (long) duk_get_top(ctx1));
	printf("final top ctx2: %ld\n", (long) duk_get_top(ctx2));
	printf("final top ctx3: %ld\n", (long) duk_get_top(ctx3));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_invalid_index);
	TEST_SAFE_CALL(test_invalid_target);
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_noeval);
	TEST_SAFE_CALL(test_regexp_literals);
	TEST_SAFE_CALL(test_regexp_prototype_shared);
	TEST_SAFE_CALL(test_set_after_thread_create);
	TEST_SAFE_CALL(test_set_before_thread_create);
}
