/*===
*** test_1a (duk_safe_call)
obj.foo -> rc=1
obj.nonexistent -> rc=0
obj[123] -> rc=1
arr.nonexistent -> rc=0
arr[2] -> rc=1
arr.length -> rc=1
final top: 3
==> rc=0, result='undefined'
*** test_1b (duk_safe_call)
==> rc=1, result='Error: invalid stack index 234'
*** test_1c (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
*** test_1d (duk_safe_call)
==> rc=1, result='TypeError: invalid base value'
*** test_1e (duk_safe_call)
==> rc=1, result='TypeError: invalid base value'
*** test_2a (duk_safe_call)
obj.foo -> rc=1
obj.nonexistent -> rc=0
obj['123'] -> rc=1
arr.nonexistent -> rc=0
arr['2'] -> rc=1
arr.length -> rc=1
final top: 3
==> rc=0, result='undefined'
*** test_2b (duk_safe_call)
==> rc=1, result='Error: invalid stack index 234'
*** test_2c (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
*** test_3a (duk_safe_call)
obj[31337] -> rc=0
obj[123] -> rc=1
arr[31337] -> rc=0
arr[2] -> rc=1
final top: 3
==> rc=0, result='undefined'
*** test_3b (duk_safe_call)
==> rc=1, result='Error: invalid stack index 234'
*** test_3c (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
===*/

static void prep(duk_context *ctx) {
	duk_set_top(ctx, 0);

	/* 0: object with both string and number keys */
	duk_push_string(ctx, "{\"foo\": \"fooval\", \"bar\": \"barval\", \"123\": \"123val\"}");
	(void) duk_json_decode(ctx, -1);

	/* 1: array with 3 elements */
	duk_push_string(ctx, "[ \"foo\", \"bar\", \"quux\" ]");
	(void) duk_json_decode(ctx, -1);

	/* 2: plain string */
	duk_push_string(ctx, "test_string");
}

/* duk_has_prop(), success cases */
static duk_ret_t test_1a(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	duk_push_string(ctx, "foo");
	rc = duk_has_prop(ctx, 0);
	printf("obj.foo -> rc=%d\n", (int) rc);

	duk_push_string(ctx, "nonexistent");
	rc = duk_has_prop(ctx, 0);
	printf("obj.nonexistent -> rc=%d\n", (int) rc);

	duk_push_int(ctx, 123);
	rc = duk_has_prop(ctx, 0);
	printf("obj[123] -> rc=%d\n", (int) rc);

	duk_push_string(ctx, "nonexistent");
	rc = duk_has_prop(ctx, 1);
	printf("arr.nonexistent -> rc=%d\n", (int) rc);

	duk_push_int(ctx, 2);
	rc = duk_has_prop(ctx, 1);
	printf("arr[2] -> rc=%d\n", (int) rc);

	duk_push_string(ctx, "length");
	rc = duk_has_prop(ctx, 1);
	printf("arr.length -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop(), invalid index */
static duk_ret_t test_1b(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	duk_push_string(ctx, "foo");
	rc = duk_has_prop(ctx, 234);
	printf("obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop(), DUK_INVALID_INDEX */
static duk_ret_t test_1c(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	duk_push_string(ctx, "foo");
	rc = duk_has_prop(ctx, DUK_INVALID_INDEX);
	printf("obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop(), not an object */
static duk_ret_t test_1d(duk_context *ctx) {
	duk_ret_t rc;

	duk_set_top(ctx, 0);

	duk_push_string(ctx, "test");
	duk_push_string(ctx, "3");
	rc = duk_has_prop(ctx, -2);
	printf("'test'['3'] -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop(), not an object */
static duk_ret_t test_1e(duk_context *ctx) {
	duk_ret_t rc;

	duk_set_top(ctx, 0);

	duk_push_null(ctx);
	duk_push_string(ctx, "foo");
	rc = duk_has_prop(ctx, -2);
	printf("null.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_string(), success cases */
static duk_ret_t test_2a(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_has_prop_string(ctx, 0, "foo");
	printf("obj.foo -> rc=%d\n", (int) rc);

	rc = duk_has_prop_string(ctx, 0, "nonexistent");
	printf("obj.nonexistent -> rc=%d\n", (int) rc);

	rc = duk_has_prop_string(ctx, 0, "123");
	printf("obj['123'] -> rc=%d\n", (int) rc);

	rc = duk_has_prop_string(ctx, 1, "nonexistent");
	printf("arr.nonexistent -> rc=%d\n", (int) rc);

	rc = duk_has_prop_string(ctx, 1, "2");
	printf("arr['2'] -> rc=%d\n", (int) rc);

	rc = duk_has_prop_string(ctx, 1, "length");
	printf("arr.length -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_string(), invalid index */
static duk_ret_t test_2b(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_has_prop_string(ctx, 234, "foo");
	printf("obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_string(), DUK_INVALID_INDEX */
static duk_ret_t test_2c(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_has_prop_string(ctx, DUK_INVALID_INDEX, "foo");
	printf("obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_index(), success cases */
static duk_ret_t test_3a(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_has_prop_index(ctx, 0, 31337);
	printf("obj[31337] -> rc=%d\n", (int) rc);

	rc = duk_has_prop_index(ctx, 0, 123);
	printf("obj[123] -> rc=%d\n", (int) rc);

	rc = duk_has_prop_index(ctx, 1, 31337);
	printf("arr[31337] -> rc=%d\n", (int) rc);

	rc = duk_has_prop_index(ctx, 1, 2);
	printf("arr[2] -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_index(), invalid index */
static duk_ret_t test_3b(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_has_prop_index(ctx, 234, 123);
	printf("obj[123] -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_has_prop_index(), DUK_INVALID_INDEX */
static duk_ret_t test_3c(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_has_prop_index(ctx, DUK_INVALID_INDEX, 123);
	printf("obj[123] -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1a);
	TEST_SAFE_CALL(test_1b);
	TEST_SAFE_CALL(test_1c);
	TEST_SAFE_CALL(test_1d);
	TEST_SAFE_CALL(test_1e);

	TEST_SAFE_CALL(test_2a);
	TEST_SAFE_CALL(test_2b);
	TEST_SAFE_CALL(test_2c);

	TEST_SAFE_CALL(test_3a);
	TEST_SAFE_CALL(test_3b);
	TEST_SAFE_CALL(test_3c);
}
