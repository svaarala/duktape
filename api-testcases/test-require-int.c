/*===
number: 123.000000 -> int: 123
number: 123.456000 -> int: 123
number: nan -> int: 0
number: -inf -> int: INT_MIN
number: inf -> int: INT_MAX
rc=0, result=undefined
rc=1, result=TypeError: not number
rc=1, result=TypeError: not number
rc=1, result=TypeError: not number
===*/

int test_1(duk_context *ctx) {
	int i;

	duk_set_top(ctx, 0);
	duk_push_int(ctx, 123);
	duk_push_number(ctx, 123.456);
	duk_push_nan(ctx);
	duk_push_number(ctx, -INFINITY);
	duk_push_number(ctx, INFINITY);

	for (i = 0; i < 5; i++) {
		int ival = duk_require_int(ctx, i);
		double dval = duk_get_number(ctx, i);

		printf("number: %lf -> int: ", dval);
		if (ival == INT_MIN) {
			printf("INT_MIN");
		} else if (ival == INT_MAX) {
			printf("INT_MAX");
		} else {
			printf("%d", ival);
		}
		printf("\n");
	}
	return 0;
}

int test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_null(ctx);
	printf("int: %d\n", duk_require_int(ctx, 0));
	return 0;
}

int test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);
	printf("int: %d\n", duk_require_int(ctx, 0));
	return 0;
}

int test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);
	printf("int: %d\n", duk_require_int(ctx, DUK_INVALID_INDEX));
	return 0;
}

void test(duk_context *ctx) {
	int rc;

	rc = duk_safe_call(ctx, test_1, 0, 1);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_2, 0, 1);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_3, 0, 1);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_4, 0, 1);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);
}

