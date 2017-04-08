/*===
positive numbers truncate towards zero
number: 3
negative numbers are clipped to zero
number: 0
above DUK_UINT_MAX
number is DUK_UINT_MAX: 1
NaN coerces to zero
number: 0
non-number coerces to zero
number: 0
invalid index returns zero
number: 0
===*/

void test(duk_context *ctx) {
	double d;

	printf("positive numbers truncate towards zero\n");
	d = 3.9;
	duk_push_number(ctx, d);
	printf("number: %lu\n", (unsigned long) duk_get_uint(ctx, -1));
	duk_pop(ctx);

	printf("negative numbers are clipped to zero\n");
	d = -3.9;
	duk_push_number(ctx, d);
	printf("number: %lu\n", (unsigned long) duk_get_uint(ctx, -1));
	duk_pop(ctx);

	printf("above DUK_UINT_MAX\n");
	d = ((double) DUK_UINT_MAX) * 2.0;
	duk_push_number(ctx, d);
	printf("number is DUK_UINT_MAX: %d\n", (duk_get_uint(ctx, -1) == DUK_UINT_MAX));
	duk_pop(ctx);

	printf("NaN coerces to zero\n");
	duk_push_nan(ctx);
	printf("number: %lu\n", (unsigned long) duk_get_uint(ctx, -1));
	duk_pop(ctx);

	printf("non-number coerces to zero\n");
	duk_push_string(ctx, "123");
	printf("number: %lu\n", (unsigned long) duk_get_uint(ctx, -1));
	duk_pop(ctx);

	printf("invalid index returns zero\n");
	printf("number: %lu\n", (unsigned long) duk_get_uint(ctx, 100));
}
