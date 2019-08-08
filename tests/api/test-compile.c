/*===
*** test_program (duk_safe_call)
program
program result: 123.000000
final top: 0
==> rc=0, result='undefined'
*** test_eval (duk_safe_call)
eval result: 5.000000
final top: 0
==> rc=0, result='undefined'
*** test_function (duk_safe_call)
function result: 11.000000
final top: 0
==> rc=0, result='undefined'
*** test_syntax_error (duk_safe_call)
compile result: SyntaxError: invalid object literal (line 3, end of input) (rc=1)
final top: 0
==> rc=0, result='undefined'
*** test_constructable_and_name (duk_safe_call)
hello
function
is object: 1
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_program(duk_context *ctx, void *udata) {
	(void) udata;

	duk_set_top(ctx, 0);

	duk_push_string(ctx, "print('program');\n"
	                     "function hello() { print('Hello world!'); }\n"
	                     "123;");
	duk_push_string(ctx, "program");
	duk_compile(ctx, 0);
	duk_call(ctx, 0);      /* [ func filename ] -> [ result ] */
	printf("program result: %lf\n", duk_get_number(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_eval(duk_context *ctx, void *udata) {
	(void) udata;

	duk_set_top(ctx, 0);

	duk_push_string(ctx, "2+3");
	duk_push_string(ctx, "eval");
	duk_compile(ctx, DUK_COMPILE_EVAL);
	duk_call(ctx, 0);      /* [ func ] -> [ result ] */
	printf("eval result: %lf\n", duk_get_number(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_function(duk_context *ctx, void *udata) {
	(void) udata;

	duk_set_top(ctx, 0);

	duk_push_string(ctx, "function (x,y) { return x+y; }");
	duk_push_string(ctx, "function");
	duk_compile(ctx, DUK_COMPILE_FUNCTION);
	duk_push_int(ctx, 5);
	duk_push_int(ctx, 6);
	duk_call(ctx, 2);      /* [ func 5 6 ] -> [ result ] */
	printf("function result: %lf\n", duk_get_number(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_syntax_error(duk_context *ctx, void *udata) {
	duk_ret_t rc;

	(void) udata;

	duk_set_top(ctx, 0);

	/* SyntaxError while compiling */

	duk_push_string(ctx, "print('program');\n"
	                     "function hello() { print('Hello world!'); }\n"
	                     "123; obj={");
	duk_push_string(ctx, "program");
	rc = duk_pcompile(ctx, 0);
	printf("compile result: %s (rc=%d)\n", duk_safe_to_string(ctx, -1), (int) rc);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_constructable_and_name(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_string(ctx, "function test() { print('hello'); print(typeof test); }");
	duk_push_string(ctx, "test.js");
	duk_compile(ctx, DUK_COMPILE_FUNCTION);
	duk_new(ctx, 0);
	printf("is object: %ld\n", (long) duk_is_object(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_program);
	TEST_SAFE_CALL(test_eval);
	TEST_SAFE_CALL(test_function);
	TEST_SAFE_CALL(test_syntax_error);
	TEST_SAFE_CALL(test_constructable_and_name);
}
