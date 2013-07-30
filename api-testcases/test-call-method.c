/*===
this binding: 'my this binding'
result=33
final top: 0
rc=0, result='undefined'
this binding: 'my this binding'
rc=1, result='TypeError: argument 2 is not a number'
===*/

int my_adder(duk_context *ctx) {
	int i, n;
	double res = 0.0;

	duk_push_this(ctx);
	printf("this binding: '%s'\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	n = duk_get_top(ctx);
	for (i = 0; i < n; i++) {
		if (!duk_is_number(ctx, i)) {
			duk_error(ctx, DUK_ERR_TYPE_ERROR, "argument %d is not a number", i);
		}
		res += duk_get_number(ctx, i);
	}

	duk_push_number(ctx, res);
	return 1;
}

int test_1(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_new_c_function(ctx, my_adder, 3 /*nargs*/);
	duk_push_string(ctx, "my this binding");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	duk_push_int(ctx, 12);
	duk_push_int(ctx, 13);  /* clipped */
	duk_push_int(ctx, 14);  /* clipped */
	duk_call_method(ctx, 5);

	printf("result=%s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

int test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_push_new_c_function(ctx, my_adder, 3 /*nargs*/);
	duk_push_string(ctx, "my this binding");
	duk_push_int(ctx, 10);
	duk_push_int(ctx, 11);
	duk_push_string(ctx, "foo");  /* causes error */
	duk_push_int(ctx, 13);  /* clipped */
	duk_push_int(ctx, 14);  /* clipped */
	duk_call_method(ctx, 5);

	printf("result=%s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	int rc;

	rc = duk_safe_call(ctx, test_1, 0, 1, DUK_INVALID_INDEX);
	printf("rc=%d, result='%s'\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_2, 0, 1, DUK_INVALID_INDEX);
	printf("rc=%d, result='%s'\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);
}
