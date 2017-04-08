/*===
*** test_basic (duk_safe_call)
top: 14
index 0: number 123.000000, FP_NAN=0, FP_INFINITE=0, FP_ZERO=0, FP_SUBNORMAL=0, FP_NORMAL=1, signbit=0
index 1: TypeError: number required, found null (stack index 1)
index 2: TypeError: number required, found true (stack index 2)
index 3: TypeError: number required, found false (stack index 3)
index 4: TypeError: number required, found 'foo' (stack index 4)
index 5: TypeError: number required, found '123' (stack index 5)
index 6: number -inf, FP_NAN=0, FP_INFINITE=1, FP_ZERO=0, FP_SUBNORMAL=0, FP_NORMAL=0, signbit=1
index 7: number -123456789.000000, FP_NAN=0, FP_INFINITE=0, FP_ZERO=0, FP_SUBNORMAL=0, FP_NORMAL=1, signbit=1
index 8: number -0.000000, FP_NAN=0, FP_INFINITE=0, FP_ZERO=1, FP_SUBNORMAL=0, FP_NORMAL=0, signbit=1
index 9: number 0.000000, FP_NAN=0, FP_INFINITE=0, FP_ZERO=1, FP_SUBNORMAL=0, FP_NORMAL=0, signbit=0
index 10: number 123456789.000000, FP_NAN=0, FP_INFINITE=0, FP_ZERO=0, FP_SUBNORMAL=0, FP_NORMAL=1, signbit=0
index 11: number inf, FP_NAN=0, FP_INFINITE=1, FP_ZERO=0, FP_SUBNORMAL=0, FP_NORMAL=0, signbit=0
index 12: number nan, FP_NAN=1, FP_INFINITE=0, FP_ZERO=0, FP_SUBNORMAL=0, FP_NORMAL=0, signbit=0
index 13: TypeError: number required, found [object Object] (stack index 13)
index 14: number 123.000000, FP_NAN=0, FP_INFINITE=0, FP_ZERO=0, FP_SUBNORMAL=0, FP_NORMAL=1, signbit=0
==> rc=0, result='undefined'
===*/

static duk_ret_t safe_helper(duk_context *ctx, void *udata) {
	duk_idx_t idx = (duk_idx_t) udata & 0xffffffffUL;
	double d = duk_opt_number(ctx, idx, 123.0);
	int c = fpclassify(d);
	(void) udata;

	printf("index %ld: number %lf, FP_NAN=%d, FP_INFINITE=%d, FP_ZERO=%d, FP_SUBNORMAL=%d, FP_NORMAL=%d, signbit=%d\n",
	       (long) idx, d, (c == FP_NAN ? 1 : 0), (c == FP_INFINITE ? 1 : 0), (c == FP_ZERO ? 1 : 0),
	       (c == FP_SUBNORMAL ? 1 : 0), (c == FP_NORMAL ? 1 : 0), (signbit(d) ? 1 : 0));
	return 0;
}

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	duk_idx_t i, n;
	duk_int_t rc;

	(void) udata;

	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_true(ctx);
	duk_push_false(ctx);
	duk_push_string(ctx, "foo");
	duk_push_string(ctx, "123");
	duk_push_number(ctx, -INFINITY);
	duk_push_number(ctx, -123456789.0);
	duk_push_number(ctx, -0.0);
	duk_push_number(ctx, +0.0);
	duk_push_number(ctx, +123456789.0);
	duk_push_number(ctx, +INFINITY);
	duk_push_nan(ctx);
	duk_push_object(ctx);

	n = duk_get_top(ctx);
	printf("top: %ld\n", (long) n);
	for (i = 0; i <= n; i++) {
		rc = duk_safe_call(ctx, safe_helper, (void *) i, 0, 1);
		if (rc != DUK_EXEC_SUCCESS) {
			printf("index %ld: %s\n", (long) i, duk_safe_to_string(ctx, -1));
		}
		duk_pop(ctx);
	}

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
