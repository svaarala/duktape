/*===
top: 14
index 0: number nan, FP_NAN=1, FP_INFINITE=0, FP_ZERO=0, FP_SUBNORMAL=0, FP_NORMAL=0, signbit=0
index 1: number nan, FP_NAN=1, FP_INFINITE=0, FP_ZERO=0, FP_SUBNORMAL=0, FP_NORMAL=0, signbit=0
index 2: number nan, FP_NAN=1, FP_INFINITE=0, FP_ZERO=0, FP_SUBNORMAL=0, FP_NORMAL=0, signbit=0
index 3: number nan, FP_NAN=1, FP_INFINITE=0, FP_ZERO=0, FP_SUBNORMAL=0, FP_NORMAL=0, signbit=0
index 4: number nan, FP_NAN=1, FP_INFINITE=0, FP_ZERO=0, FP_SUBNORMAL=0, FP_NORMAL=0, signbit=0
index 5: number nan, FP_NAN=1, FP_INFINITE=0, FP_ZERO=0, FP_SUBNORMAL=0, FP_NORMAL=0, signbit=0
index 6: number -inf, FP_NAN=0, FP_INFINITE=1, FP_ZERO=0, FP_SUBNORMAL=0, FP_NORMAL=0, signbit=1
index 7: number -123456789.000000, FP_NAN=0, FP_INFINITE=0, FP_ZERO=0, FP_SUBNORMAL=0, FP_NORMAL=1, signbit=1
index 8: number -0.000000, FP_NAN=0, FP_INFINITE=0, FP_ZERO=1, FP_SUBNORMAL=0, FP_NORMAL=0, signbit=1
index 9: number 0.000000, FP_NAN=0, FP_INFINITE=0, FP_ZERO=1, FP_SUBNORMAL=0, FP_NORMAL=0, signbit=0
index 10: number 123456789.000000, FP_NAN=0, FP_INFINITE=0, FP_ZERO=0, FP_SUBNORMAL=0, FP_NORMAL=1, signbit=0
index 11: number inf, FP_NAN=0, FP_INFINITE=1, FP_ZERO=0, FP_SUBNORMAL=0, FP_NORMAL=0, signbit=0
index 12: number nan, FP_NAN=1, FP_INFINITE=0, FP_ZERO=0, FP_SUBNORMAL=0, FP_NORMAL=0, signbit=0
index 13: number nan, FP_NAN=1, FP_INFINITE=0, FP_ZERO=0, FP_SUBNORMAL=0, FP_NORMAL=0, signbit=0
===*/

void test(duk_context *ctx) {
	duk_idx_t i, n;

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
	for (i = 0; i < n; i++) {
		double d = duk_get_number(ctx, i);
		int c = fpclassify(d);
		printf("index %ld: number %lf, FP_NAN=%d, FP_INFINITE=%d, FP_ZERO=%d, FP_SUBNORMAL=%d, FP_NORMAL=%d, signbit=%d\n",
		       (long) i, d, (c == FP_NAN ? 1 : 0), (c == FP_INFINITE ? 1 : 0), (c == FP_ZERO ? 1 : 0),
		       (c == FP_SUBNORMAL ? 1 : 0), (c == FP_NORMAL ? 1 : 0), (signbit(d) ? 1 : 0));
	}
}
