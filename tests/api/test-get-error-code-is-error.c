/*
 *  Tests for duk_get_error_code() and duk_is_error()
 */

/*===
*** test_basic (duk_safe_call)
index 0: type=1 errcode=0 is_error=0
index 1: type=2 errcode=0 is_error=0
index 2: type=3 errcode=0 is_error=0
index 3: type=3 errcode=0 is_error=0
index 4: type=4 errcode=0 is_error=0
index 5: type=4 errcode=0 is_error=0
index 6: type=4 errcode=0 is_error=0
index 7: type=4 errcode=0 is_error=0
index 8: type=5 errcode=0 is_error=0
index 9: type=5 errcode=0 is_error=0
index 10: type=6 errcode=0 is_error=0
index 11: type=6 errcode=0 is_error=0
index 12: type=7 errcode=0 is_error=0
index 13: type=7 errcode=0 is_error=0
index 14: type=7 errcode=0 is_error=0
index 15: type=7 errcode=0 is_error=0
index 16: type=8 errcode=0 is_error=0
index 17: type=8 errcode=0 is_error=0
index 18: type=6 errcode=100 is_error=1
index 19: type=6 errcode=101 is_error=1
index 20: type=6 errcode=102 is_error=1
index 21: type=6 errcode=103 is_error=1
index 22: type=6 errcode=104 is_error=1
index 23: type=6 errcode=105 is_error=1
index 24: type=6 errcode=106 is_error=1
index 25: type=6 errcode=106 is_error=1
index 26: type=6 errcode=100 is_error=1
index 27: type=6 errcode=100 is_error=1
index 28: type=6 errcode=100 is_error=1
final top: 29
==> rc=0, result='undefined'
*** test_protoloop_code (duk_safe_call)
error code: 0
==> rc=0, result='undefined'
*** test_protoloop_iserror (duk_safe_call)
is error: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t my_error_thrower(duk_context *ctx) {
	duk_pop_n(ctx, 1000);  /* APIError */
	return DUK_RET_INTERNAL_ERROR;
}

static duk_ret_t test_basic(duk_context *ctx) {
	duk_idx_t i, n;

	/*
	 *  Test values
	 */

	/* Non-error values */
	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_true(ctx);
	duk_push_false(ctx);
	duk_push_int(ctx, 0);
	duk_push_int(ctx, 1);
	duk_push_nan(ctx);
	duk_push_number(ctx, INFINITY);
	duk_push_string(ctx, "");
	duk_push_string(ctx, "foo");
	duk_push_object(ctx);
	duk_push_thread(ctx);
	duk_push_fixed_buffer(ctx, 0);
	duk_push_fixed_buffer(ctx, 1024);
	duk_push_dynamic_buffer(ctx, 0);
	duk_push_dynamic_buffer(ctx, 1024);
	duk_push_pointer(ctx, (void *) NULL);
	duk_push_pointer(ctx, (void *) 0xdeadbeef);

	/* Some errors */
	duk_eval_string(ctx, "new Error('error')");
	duk_eval_string(ctx, "new EvalError('error')");
	duk_eval_string(ctx, "new RangeError('error')");
	duk_eval_string(ctx, "new ReferenceError('error')");
	duk_eval_string(ctx, "new SyntaxError('error')");
	duk_eval_string(ctx, "new TypeError('error')");
	duk_eval_string(ctx, "new URIError('error')");
	duk_eval_string(ctx, "function MyError1() {}; MyError1.prototype = URIError.prototype; new MyError1();");
	duk_eval_string(ctx, "function MyError2() {}; MyError2.prototype = Error.prototype; new MyError2();");
	duk_eval_string(ctx, "Object.create(new MyError2());");

	/* APIError: it's used internally (and is has its own error code)
	 * but from Ecmascript point-of-view it's an Error instance, so
	 * we get DUK_ERR_ERROR for it.
	 */
	duk_push_c_function(ctx, my_error_thrower, 0);
	duk_pcall(ctx, 0);

	for (n = duk_get_top(ctx), i = 0; i < n; i++) {
		printf("index %ld: type=%ld errcode=%ld is_error=%ld\n",
		       (long) i, (long) duk_get_type(ctx, i),
		       (long) duk_get_error_code(ctx, i), (long) duk_is_error(ctx, i));
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_protoloop_code(duk_context *ctx) {
	/*
	 *  A prototype loop currently results in a DUK_ERR_NONE
	 *  instead of an error.
	 */

	duk_push_object(ctx);  /* 0 */
	duk_push_object(ctx);  /* 1 */

	duk_dup(ctx, 0);
	duk_set_prototype(ctx, 1);
	duk_dup(ctx, 1);
	duk_set_prototype(ctx, 0);

	printf("error code: %ld\n", (long) duk_get_error_code(ctx, 0));
	return 0;
}

static duk_ret_t test_protoloop_iserror(duk_context *ctx) {
	duk_push_object(ctx);  /* 0 */
	duk_push_object(ctx);  /* 1 */

	duk_dup(ctx, 0);
	duk_set_prototype(ctx, 1);
	duk_dup(ctx, 1);
	duk_set_prototype(ctx, 0);

	printf("is error: %ld\n", (long) duk_is_error(ctx, 0));
	return 0;
}
void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_protoloop_code);
	TEST_SAFE_CALL(test_protoloop_iserror);
}
