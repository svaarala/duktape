/*
 *  Once a certain Duktape/C function invocation has "checked" the value
 *  stack to contain space for at least N elements, this reserve should
 *  not be reduced without warning.
 *
 *  In Duktape 0.11.0 function calls (Ecmascript or Duktape/C) would
 *  essentially reset the "checked" stack size.  This testcase demonstrates
 *  the issue and documents the desired behavior.
 */

/*===
*** test_ecma_call_success (duk_safe_call)
hello from ecma func
duk_call: 123
final top: 10001
==> rc=0, result='undefined'
*** test_ecma_call_error (duk_safe_call)
hello from ecma func
==> rc=1, result='Error: ecma thrown'
*** test_ecma_pcall_success (duk_safe_call)
hello from ecma func
duk_pcall: rc=0, value: 123
final top: 10001
==> rc=0, result='undefined'
*** test_ecma_pcall_error (duk_safe_call)
hello from ecma func
duk_pcall: rc=1, value: Error: ecma thrown
final top: 10001
==> rc=0, result='undefined'
*** test_c_call_success (duk_safe_call)
hello from my_func_success
duk_call: 234
final top: 10001
==> rc=0, result='undefined'
*** test_c_call_error1 (duk_safe_call)
hello from my_func_error1
==> rc=1, result='Error: error thrown by my_func_error1'
*** test_c_call_error2 (duk_safe_call)
hello from my_func_error2
==> rc=1, result='URIError: uri error (rc -106)'
*** test_c_pcall_success (duk_safe_call)
hello from my_func_success
duk_pcall: rc=0, value: 234
final top: 10001
==> rc=0, result='undefined'
*** test_c_pcall_error1 (duk_safe_call)
hello from my_func_error1
duk_pcall: rc=1, value: Error: error thrown by my_func_error1
final top: 10001
==> rc=0, result='undefined'
*** test_c_pcall_error2 (duk_safe_call)
hello from my_func_error2
duk_pcall: rc=1, value: URIError: uri error (rc -106)
final top: 10001
==> rc=0, result='undefined'
*** test_safe_call_success (duk_safe_call)
hello from my_safe_func_success
duk_safe_call: rc=0, value: 123
final top: 10001
==> rc=0, result='undefined'
*** test_safe_call_error1 (duk_safe_call)
hello from my_safe_func_error1
duk_safe_call: rc=1, value: Error: error thrown by my_safe_func_error1
final top: 10001
==> rc=0, result='undefined'
*** test_safe_call_error2 (duk_safe_call)
hello from my_safe_func_error2
duk_safe_call: rc=1, value: URIError: uri error (rc -106)
final top: 10001
==> rc=0, result='undefined'
===*/

#define  STACK_REQUIRE  10000

static duk_ret_t my_func_success(duk_context *ctx) {
	printf("hello from my_func_success\n");
	duk_push_int(ctx, 234);
	return 1;
}

static duk_ret_t my_func_error1(duk_context *ctx) {
	printf("hello from my_func_error1\n");
	duk_error(ctx, 123, "error thrown by my_func_error1");
	return 0;
}

static duk_ret_t my_func_error2(duk_context *ctx) {
	printf("hello from my_func_error2\n");
	return DUK_RET_URI_ERROR;
}

static duk_ret_t my_safe_func_success(duk_context *ctx) {
	printf("hello from my_safe_func_success\n");
	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	return 2;  /* caller wants 1 ret only, so 123 is effective */
}

static duk_ret_t my_safe_func_error1(duk_context *ctx) {
	printf("hello from my_safe_func_error1\n");
	duk_error(ctx, 123, "error thrown by my_safe_func_error1");
	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	return 2;  /* caller wants 1 ret only, so 123 is effective */
}

static duk_ret_t my_safe_func_error2(duk_context *ctx) {
	printf("hello from my_safe_func_error2\n");
	duk_push_int(ctx, 123);
	duk_push_int(ctx, 234);
	return DUK_RET_URI_ERROR;
}

static duk_ret_t test_ecma_call_success(duk_context *ctx) {
	duk_idx_t i;

	/* Ensure stack space for a lot of elements. */
	duk_require_stack(ctx, STACK_REQUIRE + 1);

	/* Make a function call.  Call handling resizes the value stack
	 * back and forth.  At the end, the value stack guaranteed size
	 * must include the STACK_REQUIRE checked above.
	 */

	duk_eval_string(ctx, "(function () { print('hello from ecma func'); return 123 })");
	duk_call(ctx, 0 /*nargs*/);
	printf("duk_call: %s\n", duk_safe_to_string(ctx, -1));

	/* It would be quite awkward for user code to have no stack
	 * guarantees at this point, i.e. require the user code to
	 * call duk_require_stack(ctx, STACK_REQUIRE) again here.
	 */

	for (i = 0; i < STACK_REQUIRE; i++) {
		duk_push_int(ctx, 123);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_ecma_call_error(duk_context *ctx) {
	duk_idx_t i;

	duk_require_stack(ctx, STACK_REQUIRE + 1);

	duk_eval_string(ctx, "(function () { print('hello from ecma func'); throw Error('ecma thrown'); })");
	duk_call(ctx, 0 /*nargs*/);
	printf("duk_call: %s\n", duk_safe_to_string(ctx, -1));

	for (i = 0; i < STACK_REQUIRE; i++) {
		duk_push_int(ctx, 123);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_ecma_pcall_success(duk_context *ctx) {
	duk_idx_t i;
	duk_int_t rc;

	duk_require_stack(ctx, STACK_REQUIRE + 1);

	duk_eval_string(ctx, "(function () { print('hello from ecma func'); return 123; })");
	rc = duk_pcall(ctx, 0 /*nargs*/);
	printf("duk_pcall: rc=%ld, value: %s\n", (long) rc, duk_safe_to_string(ctx, -1));

	for (i = 0; i < STACK_REQUIRE; i++) {
		duk_push_int(ctx, 123);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_ecma_pcall_error(duk_context *ctx) {
	duk_idx_t i;
	duk_int_t rc;

	duk_require_stack(ctx, STACK_REQUIRE + 1);

	duk_eval_string(ctx, "(function () { print('hello from ecma func'); throw Error('ecma thrown'); })");
	rc = duk_pcall(ctx, 0 /*nargs*/);
	printf("duk_pcall: rc=%ld, value: %s\n", (long) rc, duk_safe_to_string(ctx, -1));

	for (i = 0; i < STACK_REQUIRE; i++) {
		duk_push_int(ctx, 123);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_c_call_success(duk_context *ctx) {
	duk_idx_t i;

	duk_require_stack(ctx, STACK_REQUIRE + 1);

	duk_push_c_function(ctx, my_func_success, 0);
	duk_call(ctx, 0 /*nargs*/);
	printf("duk_call: %s\n", duk_safe_to_string(ctx, -1));

	for (i = 0; i < STACK_REQUIRE; i++) {
		duk_push_int(ctx, 123);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_c_call_error1(duk_context *ctx) {
	duk_idx_t i;

	duk_require_stack(ctx, STACK_REQUIRE + 1);

	duk_push_c_function(ctx, my_func_error1, 0);
	duk_call(ctx, 0 /*nargs*/);
	printf("duk_call: %s\n", duk_safe_to_string(ctx, -1));

	for (i = 0; i < STACK_REQUIRE; i++) {
		duk_push_int(ctx, 123);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_c_call_error2(duk_context *ctx) {
	duk_idx_t i;

	duk_require_stack(ctx, STACK_REQUIRE + 1);

	duk_push_c_function(ctx, my_func_error2, 0);
	duk_call(ctx, 0 /*nargs*/);
	printf("duk_call: %s\n", duk_safe_to_string(ctx, -1));

	for (i = 0; i < STACK_REQUIRE; i++) {
		duk_push_int(ctx, 123);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_c_pcall_success(duk_context *ctx) {
	duk_idx_t i;
	duk_int_t rc;

	duk_require_stack(ctx, STACK_REQUIRE + 1);

	duk_push_c_function(ctx, my_func_success, 0);
	rc = duk_pcall(ctx, 0 /*nargs*/);
	printf("duk_pcall: rc=%ld, value: %s\n", (long) rc, duk_safe_to_string(ctx, -1));

	for (i = 0; i < STACK_REQUIRE; i++) {
		duk_push_int(ctx, 123);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_c_pcall_error1(duk_context *ctx) {
	duk_idx_t i;
	duk_int_t rc;

	duk_require_stack(ctx, STACK_REQUIRE + 1);

	duk_push_c_function(ctx, my_func_error1, 0);
	rc = duk_pcall(ctx, 0 /*nargs*/);
	printf("duk_pcall: rc=%ld, value: %s\n", (long) rc, duk_safe_to_string(ctx, -1));

	for (i = 0; i < STACK_REQUIRE; i++) {
		duk_push_int(ctx, 123);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_c_pcall_error2(duk_context *ctx) {
	duk_idx_t i;
	duk_int_t rc;

	duk_require_stack(ctx, STACK_REQUIRE + 1);

	duk_push_c_function(ctx, my_func_error2, 0);
	rc = duk_pcall(ctx, 0 /*nargs*/);
	printf("duk_pcall: rc=%ld, value: %s\n", (long) rc, duk_safe_to_string(ctx, -1));

	for (i = 0; i < STACK_REQUIRE; i++) {
		duk_push_int(ctx, 123);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_safe_call_success(duk_context *ctx) {
	duk_idx_t i;
	duk_int_t rc;

	duk_require_stack(ctx, STACK_REQUIRE + 1);

	rc = duk_safe_call(ctx, my_safe_func_success, 0 /*nargs*/, 1 /*nrets*/);
	printf("duk_safe_call: rc=%ld, value: %s\n", (long) rc, duk_safe_to_string(ctx, -1));

	for (i = 0; i < STACK_REQUIRE; i++) {
		duk_push_int(ctx, 123);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_safe_call_error1(duk_context *ctx) {
	duk_idx_t i;
	duk_int_t rc;

	duk_require_stack(ctx, STACK_REQUIRE + 1);

	rc = duk_safe_call(ctx, my_safe_func_error1, 0 /*nargs*/, 1 /*nrets*/);
	printf("duk_safe_call: rc=%ld, value: %s\n", (long) rc, duk_safe_to_string(ctx, -1));

	for (i = 0; i < STACK_REQUIRE; i++) {
		duk_push_int(ctx, 123);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_safe_call_error2(duk_context *ctx) {
	duk_idx_t i;
	duk_int_t rc;

	duk_require_stack(ctx, STACK_REQUIRE + 1);

	rc = duk_safe_call(ctx, my_safe_func_error2, 0 /*nargs*/, 1 /*nrets*/);
	printf("duk_safe_call: rc=%ld, value: %s\n", (long) rc, duk_safe_to_string(ctx, -1));

	for (i = 0; i < STACK_REQUIRE; i++) {
		duk_push_int(ctx, 123);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_ecma_call_success);
	TEST_SAFE_CALL(test_ecma_call_error);
	TEST_SAFE_CALL(test_ecma_pcall_success);
	TEST_SAFE_CALL(test_ecma_pcall_error);

	TEST_SAFE_CALL(test_c_call_success);
	TEST_SAFE_CALL(test_c_call_error1);
	TEST_SAFE_CALL(test_c_call_error2);
	TEST_SAFE_CALL(test_c_pcall_success);
	TEST_SAFE_CALL(test_c_pcall_error1);
	TEST_SAFE_CALL(test_c_pcall_error2);

	TEST_SAFE_CALL(test_safe_call_success);
	TEST_SAFE_CALL(test_safe_call_error1);
	TEST_SAFE_CALL(test_safe_call_error2);
}
