/*===
rc=0, result=undefined
rc=1, result=TypeError: not null
rc=1, result=TypeError: not null
rc=1, result=TypeError: not null
===*/

int test_1(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_null(ctx);
	duk_require_null(ctx, 0);
	return 0;
}

int test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_push_undefined(ctx);
	duk_require_null(ctx, 0);
	return 0;
}

int test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_require_null(ctx, 0);
	printf("require 0 OK\n");
	return 0;
}

int test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);
	duk_require_null(ctx, DUK_INVALID_INDEX);
	printf("require DUK_INVALID_INDEX OK\n");
	return 0;
}

void test(duk_context *ctx) {
	int rc;

	rc = duk_safe_call(ctx, test_1, 0, 1, DUK_INVALID_INDEX);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_2, 0, 1, DUK_INVALID_INDEX);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_3, 0, 1, DUK_INVALID_INDEX);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);

	rc = duk_safe_call(ctx, test_4, 0, 1, DUK_INVALID_INDEX);
	printf("rc=%d, result=%s\n", rc, duk_to_string(ctx, -1));
	duk_pop(ctx);
}

