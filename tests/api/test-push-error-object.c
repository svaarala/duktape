/*===
*** test_1 (duk_safe_call)
err_idx: 2
name: TypeError
message: invalid argument: 234
code: undefined
final top: 3
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
err_idx: 2
name: RangeError
message: range error: 123 234
final top: 3
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	duk_idx_t err_idx;

	/* dummy values */
	duk_push_int(ctx, 123);
	duk_push_int(ctx, 123);

	err_idx = duk_push_error_object(ctx, DUK_ERR_TYPE_ERROR, "invalid argument: %d", 234);
	printf("err_idx: %ld\n", (long) err_idx);

	duk_get_prop_string(ctx, -1, "name");
	printf("name: %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "message");
	printf("message: %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	/* 'code' was a property which was once augmented to error instances,
	 * but has since been removed.
	 */
	duk_get_prop_string(ctx, -1, "code");
	printf("code: %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Vararg variants */
static void push_1(duk_context *ctx, duk_errcode_t errcode, const char *fmt, ...) {
	duk_idx_t err_idx;
	va_list ap;

	va_start(ap, fmt);

	err_idx = duk_push_error_object_va(ctx, errcode, fmt, ap);
	printf("err_idx: %ld\n", (long) err_idx);

	duk_get_prop_string(ctx, -1, "name");
	printf("name: %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "message");
	printf("message: %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	va_end(ap);
}

static duk_ret_t test_2(duk_context *ctx) {
	/* dummy values */
	duk_push_int(ctx, 123);
	duk_push_int(ctx, 123);

	push_1(ctx, DUK_ERR_RANGE_ERROR, "range error: %d %d", 123, 234);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
}
