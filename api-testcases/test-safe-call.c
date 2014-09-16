/*===
1st return value: 21
2nd return value: undefined
error: Error: test_2 error
final top: 1
===*/

static duk_ret_t test_1(duk_context *ctx) {
	double a, b, c;

	a = duk_get_number(ctx, -3);
	b = duk_get_number(ctx, -2);
	c = duk_get_number(ctx, -1);
	c = c;  /* silence warning */
	duk_push_number(ctx, a + b);

	/* just one return value */
	return 1;
}

static duk_ret_t test_2(duk_context *ctx) {
	duk_error(ctx, DUK_ERR_INTERNAL_ERROR, "test_2 error");
	return 0;
}

void test(duk_context *ctx) {
	duk_ret_t rc;

	duk_set_top(ctx, 0);

	duk_push_string(ctx, "foo");  /* dummy */

	/* success case */
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	duk_push_int(ctx, 12);
	rc = duk_safe_call(ctx, test_1, 3 /*nargs*/, 2 /*nrets*/);
	if (rc == DUK_EXEC_SUCCESS) {
		printf("1st return value: %s\n", duk_to_string(ctx, -2));  /* 21 */
		printf("2nd return value: %s\n", duk_to_string(ctx, -1));  /* undefined */
	} else {
		printf("error: %s\n", duk_to_string(ctx, -2));
	}
	duk_pop_2(ctx);

	/* error case */
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	duk_push_int(ctx, 12);
	rc = duk_safe_call(ctx, test_2, 3 /*nargs*/, 2 /*nrets*/);
	if (rc == DUK_EXEC_SUCCESS) {
		printf("1st return value: %s\n", duk_to_string(ctx, -2));  /* 21 */
		printf("2nd return value: %s\n", duk_to_string(ctx, -1));  /* undefined */
	} else {
		printf("error: %s\n", duk_to_string(ctx, -2));
	}
	duk_pop_2(ctx);

	/* XXX: also test invalid input stack shapes (like not enough args) */

	printf("final top: %ld\n", (long) duk_get_top(ctx));
}
