/*===
rc=1 -> throw me
===*/

int test_1(duk_context *ctx) {
	duk_push_string(ctx, "throw me");
	duk_throw(ctx);
	return 0;
}

void test(duk_context *ctx) {
	int rc;

	rc = duk_safe_call(ctx, test_1, 0, 1);
	printf("rc=%d -> %s\n", rc, duk_get_string(ctx, -1));
}

