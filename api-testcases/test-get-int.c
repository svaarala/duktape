/*===
positive numbers truncate towards zero
number: 3
negative numbers truncate towards zero
number: -3
integer below INT_MIN, coerces to INT_MIN
number is INT_MIN: 1
INT_MIN
number is INT_MIN: 1
INT_MAX
number is INT_MAX: 1
integer above INT_MAX, coerces to INT_MAX
number is INT_MAX: 1
NaN coerces to zero
number: 0
non-number coerces to zero
number: 0
===*/

void test(duk_context *ctx) {
	double d;

	/* INT_MIN/INT_MAX cases don't print concrete numbers because
	 * the limits are platform dependent.  In particular, on platforms
	 * with 64-bit ints we want to allow the full range.
	 */

	printf("positive numbers truncate towards zero\n");
	d = 3.9;
	duk_push_number(ctx, d);
	printf("number: %d\n", duk_get_int(ctx, -1));
	duk_pop(ctx);

	printf("negative numbers truncate towards zero\n");
	d = -3.9;
	duk_push_number(ctx, d);
	printf("number: %d\n", duk_get_int(ctx, -1));
	duk_pop(ctx);

	printf("integer below INT_MIN, coerces to INT_MIN\n");
	d = ((double) INT_MIN) - 1.0;
	duk_push_number(ctx, d);
#if 0
	printf("number: %d\n", duk_get_int(ctx, -1));
#endif
	printf("number is INT_MIN: %d\n", (duk_get_int(ctx, -1) == INT_MIN));
	duk_pop(ctx);

	printf("INT_MIN\n");
	d = (double) INT_MIN;
	duk_push_number(ctx, d);
#if 0
	printf("number: %d\n", duk_get_int(ctx, -1));
#endif
	printf("number is INT_MIN: %d\n", (duk_get_int(ctx, -1) == INT_MIN));
	duk_pop(ctx);

	printf("INT_MAX\n");
	d = (double) INT_MAX;
	duk_push_number(ctx, d);
#if 0
	printf("number: %d\n", duk_get_int(ctx, -1));
#endif
	printf("number is INT_MAX: %d\n", (duk_get_int(ctx, -1) == INT_MAX));
	duk_pop(ctx);

	printf("integer above INT_MAX, coerces to INT_MAX\n");
	d = ((double) INT_MAX) + 1.0;
	duk_push_number(ctx, d);
#if 0
	printf("number: %d\n", duk_get_int(ctx, -1));
#endif
	printf("number is INT_MAX: %d\n", (duk_get_int(ctx, -1) == INT_MAX));
	duk_pop(ctx);

	printf("NaN coerces to zero\n");
	duk_push_nan(ctx);
	printf("number: %d\n", duk_get_int(ctx, -1));
	duk_pop(ctx);

	printf("non-number coerces to zero\n");
	duk_push_string(ctx, "123");
	printf("number: %d\n", duk_get_int(ctx, -1));
	duk_pop(ctx);
}

