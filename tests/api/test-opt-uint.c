/*===
positive numbers truncate towards zero
number: 3
negative numbers are clipped to zero
number: 0
above DUK_UINT_MAX
number is DUK_UINT_MAX: 1
NaN coerces to zero
number: 0
non-number returns default value
number: 123
invalid index returns default value
number: 123
===*/

void test(duk_context *ctx) {
	double d;

	printf("positive numbers truncate towards zero\n");
	d = 3.9;
	duk_push_number(ctx, d);
	printf("number: %lu\n", (unsigned long) duk_opt_uint(ctx, -1, 123));
	duk_pop(ctx);

	printf("negative numbers are clipped to zero\n");
	d = -3.9;
	duk_push_number(ctx, d);
	printf("number: %lu\n", (unsigned long) duk_opt_uint(ctx, -1, 123));
	duk_pop(ctx);

	printf("above DUK_UINT_MAX\n");
	d = ((double) DUK_UINT_MAX) * 2.0;
	duk_push_number(ctx, d);
	printf("number is DUK_UINT_MAX: %d\n", (duk_opt_uint(ctx, -1, 123) == DUK_UINT_MAX));
	duk_pop(ctx);

	printf("NaN coerces to zero\n");
	duk_push_nan(ctx);
	printf("number: %lu\n", (unsigned long) duk_opt_uint(ctx, -1, 123));
	duk_pop(ctx);

	printf("non-number returns default value\n");
	duk_push_string(ctx, "123");
	printf("number: %lu\n", (unsigned long) duk_opt_uint(ctx, -1, 123));
	duk_pop(ctx);

	printf("invalid index returns default value\n");
	printf("number: %lu\n", (unsigned long) duk_opt_uint(ctx, 100, 123));
}
