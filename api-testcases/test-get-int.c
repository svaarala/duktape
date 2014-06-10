/*===
positive numbers truncate towards zero
number: 3
negative numbers truncate towards zero
number: -3
below DUK_INT_MIN
number is DUK_INT_MIN: 1
above DUK_INT_MAX
number is DUK_INT_MAX: 1
NaN coerces to zero
number: 0
non-number coerces to zero
number: 0
===*/

void test(duk_context *ctx) {
	double d;

	/* DUK_INT_MIN/DUK_INT_MAX cases don't print concrete numbers because
	 * the limits are platform dependent.  In particular, on platforms
	 * with 64-bit ints we want to allow the full range.
	 */

	printf("positive numbers truncate towards zero\n");
	d = 3.9;
	duk_push_number(ctx, d);
	printf("number: %ld\n", (long) duk_get_int(ctx, -1));
	duk_pop(ctx);

	printf("negative numbers truncate towards zero\n");
	d = -3.9;
	duk_push_number(ctx, d);
	printf("number: %ld\n", (long) duk_get_int(ctx, -1));
	duk_pop(ctx);

	/* On 64-bit platforms DUK_INT_MIN, DUK_INT_MAX, DUK_UINT_MAX
	 * cannot be represented accurately with IEEE doubles, so we
	 * can't test for exact limit here.  For simplicity, just double
	 * the limit.
	 */

	printf("below DUK_INT_MIN\n");
	d = ((double) DUK_INT_MIN) * 2.0;
	duk_push_number(ctx, d);
#if 0
	printf("number: %d\n", duk_get_int(ctx, -1));
#endif
	printf("number is DUK_INT_MIN: %d\n", (duk_get_int(ctx, -1) == DUK_INT_MIN));
	duk_pop(ctx);

	printf("above DUK_INT_MAX\n");
	d = ((double) DUK_INT_MAX) * 2.0;
	duk_push_number(ctx, d);
#if 0
	printf("number: %d\n", duk_get_int(ctx, -1));
#endif
	printf("number is DUK_INT_MAX: %d\n", (duk_get_int(ctx, -1) == DUK_INT_MAX));
	duk_pop(ctx);

	printf("NaN coerces to zero\n");
	duk_push_nan(ctx);
	printf("number: %ld\n", (long) duk_get_int(ctx, -1));
	duk_pop(ctx);

	printf("non-number coerces to zero\n");
	duk_push_string(ctx, "123");
	printf("number: %ld\n", (long) duk_get_int(ctx, -1));
	duk_pop(ctx);
}
