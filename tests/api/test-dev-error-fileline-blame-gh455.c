/*
 *  Test error .fileName / .lineNumber blaming.
 *
 *  Try to cover all the C code paths.  Must test with and without tracebacks
 *  separately, as the code paths are different.
 */

/*===
*** test_empty_1 (duk_safe_call)
dummy_source.js 4
final top: 1
==> rc=0, result='undefined'
*** test_empty_2 (duk_safe_call)
dummy.c 1234
final top: 1
==> rc=0, result='undefined'
*** test_empty_3 (duk_safe_call)
dummy.c 2345
final top: 1
==> rc=0, result='undefined'
*** test_empty_4 (duk_safe_call)
undefined undefined
final top: 1
==> rc=0, result='undefined'
*** test_nofile_1 (duk_safe_call)
delete: true
dummy_source.js 4
final top: 1
==> rc=0, result='undefined'
*** test_nofile_2 (duk_safe_call)
delete: true
dummy.c 1234
final top: 1
==> rc=0, result='undefined'
*** test_nofile_3 (duk_safe_call)
delete: true
dummy.c 2345
final top: 1
==> rc=0, result='undefined'
*** test_nofile_4 (duk_safe_call)
delete: true
undefined undefined
final top: 1
==> rc=0, result='undefined'
*** test_havefile1_1 (duk_safe_call)
dummy_source.js 4
final top: 1
==> rc=0, result='undefined'
*** test_havefile1_2 (duk_safe_call)
dummy.c 1234
final top: 1
==> rc=0, result='undefined'
*** test_havefile1_3 (duk_safe_call)
dummy.c 2345
final top: 1
==> rc=0, result='undefined'
*** test_havefile1_4 (duk_safe_call)
dummy_filename.js 2
final top: 1
==> rc=0, result='undefined'
*** test_havefile2_1 (duk_safe_call)
delete: true
dummy_source.js 4
final top: 1
==> rc=0, result='undefined'
*** test_havefile2_2 (duk_safe_call)
delete: true
dummy.c 1234
final top: 1
==> rc=0, result='undefined'
*** test_havefile2_3 (duk_safe_call)
delete: true
dummy.c 2345
final top: 1
==> rc=0, result='undefined'
*** test_havefile2_4 (duk_safe_call)
delete: true
dummy_filename.c 0
final top: 1
==> rc=0, result='undefined'
*** test_deep_1a (duk_safe_call)
delete: true
target depth: 9
dummy_source.js 4
final top: 1
==> rc=0, result='undefined'
*** test_deep_1b (duk_safe_call)
delete: true
target depth: 10
dummy_source.js 4
final top: 1
==> rc=0, result='undefined'
*** test_deep_1c (duk_safe_call)
delete: true
target depth: 11
dummy_source.js 4
final top: 1
==> rc=0, result='undefined'
*** test_deep_1d (duk_safe_call)
delete: true
target depth: 50
dummy_source.js 4
final top: 1
==> rc=0, result='undefined'
*** test_deep_2a (duk_safe_call)
delete: true
target depth: 9
dummy.c 1234
final top: 1
==> rc=0, result='undefined'
*** test_deep_2b (duk_safe_call)
delete: true
target depth: 10
dummy.c 1234
final top: 1
==> rc=0, result='undefined'
*** test_deep_2c (duk_safe_call)
delete: true
target depth: 11
dummy.c 1234
final top: 1
==> rc=0, result='undefined'
*** test_deep_2d (duk_safe_call)
delete: true
target depth: 50
dummy.c 1234
final top: 1
==> rc=0, result='undefined'
*** test_deep_3a (duk_safe_call)
delete: true
target depth: 9
dummy.c 2345
final top: 1
==> rc=0, result='undefined'
*** test_deep_3b (duk_safe_call)
delete: true
target depth: 10
dummy.c 2345
final top: 1
==> rc=0, result='undefined'
*** test_deep_3c (duk_safe_call)
delete: true
target depth: 11
dummy.c 2345
final top: 1
==> rc=0, result='undefined'
*** test_deep_3d (duk_safe_call)
delete: true
target depth: 50
dummy.c 2345
final top: 1
==> rc=0, result='undefined'
*** test_deep_4a (duk_safe_call)
delete: true
target depth: 9
outer_limits.c 0
final top: 1
==> rc=0, result='undefined'
*** test_deep_4b (duk_safe_call)
delete: true
target depth: 10
outer_limits.c 0
final top: 1
==> rc=0, result='undefined'
*** test_deep_4c (duk_safe_call)
delete: true
target depth: 11
undefined undefined
final top: 1
==> rc=0, result='undefined'
*** test_deep_4d (duk_safe_call)
delete: true
target depth: 50
undefined undefined
final top: 1
==> rc=0, result='undefined'
===*/

/*
 *  Helpers
 */

static duk_c_function target_func_hack;
static int depth_hack;

static duk_ret_t my_thrower_1(duk_context *ctx) {
	/* When an error is thrown during compilation, the source file/line
	 * is always blamed.
	 */
	duk_push_string(ctx, "\n\n\nfoo=");
	duk_push_string(ctx, "dummy_source.js");
	duk_compile(ctx, DUK_COMPILE_EVAL);
	duk_call(ctx, 0);
	return 0;
}

static duk_ret_t my_thrower_2(duk_context *ctx) {
	/* When an error is thrown using duk_error(), the __FILE__ and __LINE__
	 * of the throw site gets blamed for the error w.r.t. to .fileName and
	 * .lineNumber.
	 */
#line 1234 "dummy.c"
	duk_error(ctx, DUK_ERR_RANGE_ERROR, "user error");
	return 0;
}

static duk_ret_t my_thrower_3(duk_context *ctx) {
	/* When an error is constructed using duk_push_error_object() and then
	 * thrown, the same thing happens as with duk_error().
	 */
#line 2345 "dummy.c"
	duk_push_error_object(ctx, DUK_ERR_RANGE_ERROR, "user error");
	duk_throw(ctx);
	return 0;
}

static duk_ret_t my_thrower_4(duk_context *ctx) {
	/* When an error is thrown from inside Duktape (which is always
	 * considered "infrastructure code") the __FILE__ and __LINE__
	 * are recorded in the traceback but not blamed as file/line.
	 */
	duk_push_undefined(ctx);
	duk_require_string(ctx, -1);
	return 0;
}

/*
 *  Empty callstack
 */

static duk_ret_t empty_helper(duk_context *ctx, duk_c_function target_func) {
	duk_int_t rc;

	rc = duk_safe_call(ctx, target_func, 0, 1);
	(void) rc;

	duk_eval_string(ctx, "(function (e) { print(e.fileName, e.lineNumber); if (PRINT_STACK) { print(e.stack); } })");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_empty_1(duk_context *ctx) {
	return empty_helper(ctx, my_thrower_1);
}

static duk_ret_t test_empty_2(duk_context *ctx) {
	return empty_helper(ctx, my_thrower_2);
}

static duk_ret_t test_empty_3(duk_context *ctx) {
	return empty_helper(ctx, my_thrower_3);
}

static duk_ret_t test_empty_4(duk_context *ctx) {
	return empty_helper(ctx, my_thrower_4);
}

/*
 *  Callstack has entries but nothing with a .fileName.
 */

static duk_ret_t nofile_helper_2(duk_context *ctx) {
	duk_push_string(ctx,
		"(function () {\n"
		"    var fn = function noFileName(v) { v(); return 123; };\n"
		"    print('delete: ' + delete fn.fileName);\n"
		"    return fn;\n"
		"})()");
	duk_push_string(ctx, "dummy_filename.js");
	duk_compile(ctx, DUK_COMPILE_EVAL);
	duk_call(ctx, 0);

	duk_push_c_function(ctx, target_func_hack, 0);
	duk_call(ctx, 1);
	return 0;
}

static duk_ret_t nofile_helper(duk_context *ctx, duk_c_function target_func) {
	duk_int_t rc;

	target_func_hack = target_func;
	duk_push_c_function(ctx, nofile_helper_2, 0);  /* Duktape/C func with no .fileName */
	duk_pcall(ctx, 0);
	(void) rc;

	duk_eval_string(ctx, "(function (e) { print(e.fileName, e.lineNumber); if (PRINT_STACK) { print(e.stack); } })");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_nofile_1(duk_context *ctx) {
	return nofile_helper(ctx, my_thrower_1);
}

static duk_ret_t test_nofile_2(duk_context *ctx) {
	return nofile_helper(ctx, my_thrower_2);
}

static duk_ret_t test_nofile_3(duk_context *ctx) {
	return nofile_helper(ctx, my_thrower_3);
}

static duk_ret_t test_nofile_4(duk_context *ctx) {
	return nofile_helper(ctx, my_thrower_4);
}

/*
 *  Callstack has entries with .fileName, but the innermost function
 *  does not have a filename.
 */

static duk_ret_t havefile1_helper_2(duk_context *ctx) {
	duk_push_string(ctx,
		"(function () {\n"
		"    var fn = function haveFileName(v) { v(); return 123; };\n"
		"    return fn;\n"
		"})()");
	duk_push_string(ctx, "dummy_filename.js");
	duk_compile(ctx, DUK_COMPILE_EVAL);
	duk_call(ctx, 0);

	duk_push_c_function(ctx, target_func_hack, 0);
	duk_call(ctx, 1);
	return 0;
}

static duk_ret_t havefile1_helper(duk_context *ctx, duk_c_function target_func) {
	duk_int_t rc;

	target_func_hack = target_func;
	duk_push_c_function(ctx, havefile1_helper_2, 0);  /* Duktape/C func with no .fileName */
	duk_pcall(ctx, 0);
	(void) rc;

	duk_eval_string(ctx, "(function (e) { print(e.fileName, e.lineNumber); if (PRINT_STACK) { print(e.stack); } })");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_havefile1_1(duk_context *ctx) {
	return havefile1_helper(ctx, my_thrower_1);
}

static duk_ret_t test_havefile1_2(duk_context *ctx) {
	return havefile1_helper(ctx, my_thrower_2);
}

static duk_ret_t test_havefile1_3(duk_context *ctx) {
	return havefile1_helper(ctx, my_thrower_3);
}

static duk_ret_t test_havefile1_4(duk_context *ctx) {
	return havefile1_helper(ctx, my_thrower_4);
}

/*
 *  Callstack has entries with .fileName, and the innermost function
 *  also has a filename.
 */

static duk_ret_t havefile2_helper_2(duk_context *ctx) {
	duk_push_string(ctx,
		"(function () {\n"
		"    var fn = function noFileName(v) { v(); return 123; };\n"
		"    print('delete: ' + delete fn.fileName);\n"
		"    return fn;\n"
		"})()");
	duk_push_string(ctx, "dummy_filename.js");
	duk_compile(ctx, DUK_COMPILE_EVAL);
	duk_call(ctx, 0);

	duk_push_c_function(ctx, target_func_hack, 0);
	duk_push_string(ctx, "fileName");
	duk_push_string(ctx, "dummy_filename.c");
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
	duk_call(ctx, 1);
	return 0;
}

static duk_ret_t havefile2_helper(duk_context *ctx, duk_c_function target_func) {
	duk_int_t rc;

	target_func_hack = target_func;
	duk_push_c_function(ctx, havefile2_helper_2, 0);  /* Duktape/C func with no .fileName */
	duk_pcall(ctx, 0);
	(void) rc;

	duk_eval_string(ctx, "(function (e) { print(e.fileName, e.lineNumber); if (PRINT_STACK) { print(e.stack); } })");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_havefile2_1(duk_context *ctx) {
	return havefile2_helper(ctx, my_thrower_1);
}

static duk_ret_t test_havefile2_2(duk_context *ctx) {
	return havefile2_helper(ctx, my_thrower_2);
}

static duk_ret_t test_havefile2_3(duk_context *ctx) {
	return havefile2_helper(ctx, my_thrower_3);
}

static duk_ret_t test_havefile2_4(duk_context *ctx) {
	return havefile2_helper(ctx, my_thrower_4);
}

/*
 *  Callstack has entries with .fileName but those entries are deeper than
 *  the traceback depth so that they don't get blamed.
 *
 *  The default callstack depth is 10, so test boundary values 9, 10, 11,
 *  and a much deeper 50.
 */

static duk_ret_t deep_helper_2(duk_context *ctx) {
	duk_push_string(ctx,
		"(function () {\n"
		"    var fn = function noFileName(n, v) { if (n > 0) { noFileName(n - 1, v); } else { v(); } return 123; };\n"
		"    print('delete: ' + delete fn.fileName);\n"
		"    return fn;\n"
		"})()");
	duk_push_string(ctx, "dummy_filename.js");
	duk_compile(ctx, DUK_COMPILE_EVAL);
	duk_call(ctx, 0);

	printf("target depth: %d\n", (int) depth_hack);
	duk_push_int(ctx, depth_hack - 3);  /* account for: one func already in callstack; first call into the helper; final call to target */
	duk_push_c_function(ctx, target_func_hack, 0);
	duk_call(ctx, 2);
	return 0;
}

static duk_ret_t deep_helper(duk_context *ctx, duk_c_function target_func, int depth) {
	duk_int_t rc;

	target_func_hack = target_func;
	depth_hack = depth;
	duk_push_c_function(ctx, deep_helper_2, 0);  /* Duktape/C func with .fileName */
	duk_push_string(ctx, "fileName");
	duk_push_string(ctx, "outer_limits.c");
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE | DUK_DEFPROP_SET_WRITABLE | DUK_DEFPROP_SET_CONFIGURABLE);
	duk_pcall(ctx, 0);
	(void) rc;

	duk_eval_string(ctx, "(function (e) { print(e.fileName, e.lineNumber); if (PRINT_STACK) { print(e.stack); } })");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_deep_1a(duk_context *ctx) {
	return deep_helper(ctx, my_thrower_1, 9);
}
static duk_ret_t test_deep_1b(duk_context *ctx) {
	return deep_helper(ctx, my_thrower_1, 10);
}
static duk_ret_t test_deep_1c(duk_context *ctx) {
	return deep_helper(ctx, my_thrower_1, 11);
}
static duk_ret_t test_deep_1d(duk_context *ctx) {
	return deep_helper(ctx, my_thrower_1, 50);
}

static duk_ret_t test_deep_2a(duk_context *ctx) {
	return deep_helper(ctx, my_thrower_2, 9);
}
static duk_ret_t test_deep_2b(duk_context *ctx) {
	return deep_helper(ctx, my_thrower_2, 10);
}
static duk_ret_t test_deep_2c(duk_context *ctx) {
	return deep_helper(ctx, my_thrower_2, 11);
}
static duk_ret_t test_deep_2d(duk_context *ctx) {
	return deep_helper(ctx, my_thrower_2, 50);
}

static duk_ret_t test_deep_3a(duk_context *ctx) {
	return deep_helper(ctx, my_thrower_3, 9);
}
static duk_ret_t test_deep_3b(duk_context *ctx) {
	return deep_helper(ctx, my_thrower_3, 10);
}
static duk_ret_t test_deep_3c(duk_context *ctx) {
	return deep_helper(ctx, my_thrower_3, 11);
}
static duk_ret_t test_deep_3d(duk_context *ctx) {
	return deep_helper(ctx, my_thrower_3, 50);
}

static duk_ret_t test_deep_4a(duk_context *ctx) {
	return deep_helper(ctx, my_thrower_4, 9);
}
static duk_ret_t test_deep_4b(duk_context *ctx) {
	return deep_helper(ctx, my_thrower_4, 10);
}
static duk_ret_t test_deep_4c(duk_context *ctx) {
	return deep_helper(ctx, my_thrower_4, 11);
}
static duk_ret_t test_deep_4d(duk_context *ctx) {
	return deep_helper(ctx, my_thrower_4, 50);
}

/*
 *  User code can set an explicit .fileName and .lineNumber on the error
 *  object to avoid this default blaming.
 */

void test(duk_context *ctx) {
	/* For manual testing: */
#if 0
	duk_eval_string_noresult(ctx, "this.PRINT_STACK = true;");
#else
	duk_eval_string_noresult(ctx, "this.PRINT_STACK = false;");
#endif

	TEST_SAFE_CALL(test_empty_1);
	TEST_SAFE_CALL(test_empty_2);
	TEST_SAFE_CALL(test_empty_3);
	TEST_SAFE_CALL(test_empty_4);

	TEST_SAFE_CALL(test_nofile_1);
	TEST_SAFE_CALL(test_nofile_2);
	TEST_SAFE_CALL(test_nofile_3);
	TEST_SAFE_CALL(test_nofile_4);

	TEST_SAFE_CALL(test_havefile1_1);
	TEST_SAFE_CALL(test_havefile1_2);
	TEST_SAFE_CALL(test_havefile1_3);
	TEST_SAFE_CALL(test_havefile1_4);

	TEST_SAFE_CALL(test_havefile2_1);
	TEST_SAFE_CALL(test_havefile2_2);
	TEST_SAFE_CALL(test_havefile2_3);
	TEST_SAFE_CALL(test_havefile2_4);

	TEST_SAFE_CALL(test_deep_1a);
	TEST_SAFE_CALL(test_deep_1b);
	TEST_SAFE_CALL(test_deep_1c);
	TEST_SAFE_CALL(test_deep_1d);
	TEST_SAFE_CALL(test_deep_2a);
	TEST_SAFE_CALL(test_deep_2b);
	TEST_SAFE_CALL(test_deep_2c);
	TEST_SAFE_CALL(test_deep_2d);
	TEST_SAFE_CALL(test_deep_3a);
	TEST_SAFE_CALL(test_deep_3b);
	TEST_SAFE_CALL(test_deep_3c);
	TEST_SAFE_CALL(test_deep_3d);
	TEST_SAFE_CALL(test_deep_4a);
	TEST_SAFE_CALL(test_deep_4b);
	TEST_SAFE_CALL(test_deep_4c);
	TEST_SAFE_CALL(test_deep_4d);
}
