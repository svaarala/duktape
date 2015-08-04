/*===
*** test_1a (duk_safe_call)
delete obj.foo -> rc=1
delete obj.nonexistent -> rc=1
delete obj[123] -> rc=1
delete arr.nonexistent -> rc=1
delete arr[2] -> rc=1
final object: {"bar":"barval"}
final array: ["foo","bar",null]
final top: 3
==> rc=0, result='undefined'
*** test_1b (duk_safe_call)
==> rc=1, result='TypeError: not configurable'
*** test_1b (duk_pcall)
==> rc=1, result='TypeError: not configurable'
*** test_1c (duk_safe_call)
==> rc=1, result='TypeError: not configurable'
*** test_1c (duk_pcall)
==> rc=1, result='TypeError: not configurable'
*** test_1d (duk_safe_call)
==> rc=1, result='TypeError: not configurable'
*** test_1d (duk_pcall)
==> rc=1, result='TypeError: not configurable'
*** test_1e (duk_safe_call)
==> rc=1, result='Error: invalid index'
*** test_1e (duk_pcall)
==> rc=1, result='Error: invalid index'
*** test_1f (duk_safe_call)
==> rc=1, result='Error: invalid index'
*** test_1f (duk_pcall)
==> rc=1, result='Error: invalid index'
*** test_1g (duk_safe_call)
==> rc=1, result='TypeError: invalid base value'
*** test_1g (duk_pcall)
==> rc=1, result='TypeError: invalid base value'
*** test_2a (duk_safe_call)
delete obj.foo -> rc=1
delete obj.nonexistent -> rc=1
delete obj['123'] -> rc=1
delete arr.nonexistent -> rc=1
delete arr['2'] -> rc=1
final object: {"bar":"barval"}
final array: ["foo","bar",null]
final top: 3
==> rc=0, result='undefined'
*** test_2b (duk_safe_call)
==> rc=1, result='TypeError: not configurable'
*** test_2b (duk_pcall)
==> rc=1, result='TypeError: not configurable'
*** test_2c (duk_safe_call)
==> rc=1, result='TypeError: not configurable'
*** test_2c (duk_pcall)
==> rc=1, result='TypeError: not configurable'
*** test_2d (duk_safe_call)
==> rc=1, result='TypeError: not configurable'
*** test_2d (duk_pcall)
==> rc=1, result='TypeError: not configurable'
*** test_2e (duk_safe_call)
==> rc=1, result='Error: invalid index'
*** test_2e (duk_pcall)
==> rc=1, result='Error: invalid index'
*** test_2f (duk_safe_call)
==> rc=1, result='Error: invalid index'
*** test_2f (duk_pcall)
==> rc=1, result='Error: invalid index'
*** test_2g (duk_safe_call)
==> rc=1, result='TypeError: invalid base value'
*** test_2g (duk_pcall)
==> rc=1, result='TypeError: invalid base value'
*** test_3a (duk_safe_call)
delete obj[31337] -> rc=1
delete obj[123] -> rc=1
delete arr[31337] -> rc=1
delete arr[2] -> rc=1
final object: {"foo":"fooval","bar":"barval"}
final array: ["foo","bar",null]
final top: 3
==> rc=0, result='undefined'
*** test_3b (duk_safe_call)
==> rc=1, result='TypeError: not configurable'
*** test_3b (duk_pcall)
==> rc=1, result='TypeError: not configurable'
*** test_3c (duk_safe_call)
==> rc=1, result='Error: invalid index'
*** test_3c (duk_pcall)
==> rc=1, result='Error: invalid index'
*** test_3d (duk_safe_call)
==> rc=1, result='Error: invalid index'
*** test_3d (duk_pcall)
==> rc=1, result='Error: invalid index'
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

/* duk_del_prop(), success cases */
static duk_ret_t test_1a(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	/* existing, configurable */
	duk_push_string(ctx, "foo");
	rc = duk_del_prop(ctx, 0);
	printf("delete obj.foo -> rc=%d\n", (int) rc);

	/* nonexistent */
	duk_push_string(ctx, "nonexistent");
	rc = duk_del_prop(ctx, 0);
	printf("delete obj.nonexistent -> rc=%d\n", (int) rc);

	/* nonexistent */
	duk_push_int(ctx, 123);
	rc = duk_del_prop(ctx, 0);
	printf("delete obj[123] -> rc=%d\n", (int) rc);

	/* nonexistent, array */
	duk_push_string(ctx, "nonexistent");
	rc = duk_del_prop(ctx, 1);
	printf("delete arr.nonexistent -> rc=%d\n", (int) rc);

	/* existing, configurable, array */
	duk_push_int(ctx, 2);
	rc = duk_del_prop(ctx, 1);
	printf("delete arr[2] -> rc=%d\n", (int) rc);

	duk_json_encode(ctx, 0);
	printf("final object: %s\n", duk_to_string(ctx, 0));
	duk_json_encode(ctx, 1);
	printf("final array: %s\n", duk_to_string(ctx, 1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop(), non-configurable property (array 'length' property).
 * Same behavior when called inside/outside of a Duktape/C activation
 * (since Duktape 0.12.0 both cases are considered strict).
 */
static duk_ret_t test_1b(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	duk_push_string(ctx, "length");
	rc = duk_del_prop(ctx, 1);
	printf("delete arr.length -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop(), non-configurable virtual property of a plain string.
 * Same behavior when called inside/outside of a Duktape/C activation.
 */
static duk_ret_t test_1c(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	duk_push_int(ctx, 5);
	rc = duk_del_prop(ctx, 2);
	printf("delete 'test_string'[5] -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop(), non-configurable virtual property of a plain string.
 * Same behavior when called inside/outside of a Duktape/C activation.
 */
static duk_ret_t test_1d(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	duk_push_string(ctx, "length");
	rc = duk_del_prop(ctx, 2);
	printf("delete 'test_string'.length -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop(), invalid index */
static duk_ret_t test_1e(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	duk_push_string(ctx, "foo");
	rc = duk_del_prop(ctx, 234);
	printf("delete obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop(), DUK_INVALID_INDEX */
static duk_ret_t test_1f(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	duk_push_string(ctx, "foo");
	rc = duk_del_prop(ctx, DUK_INVALID_INDEX);
	printf("delete obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop(), not object coercible */
static duk_ret_t test_1g(duk_context *ctx) {
	duk_ret_t rc;

	duk_set_top(ctx, 0);
	duk_push_null(ctx);
	duk_push_string(ctx, "foo");
	rc = duk_del_prop(ctx, -2);
	printf("delete null.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_string(), success cases */
static duk_ret_t test_2a(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_del_prop_string(ctx, 0, "foo");
	printf("delete obj.foo -> rc=%d\n", (int) rc);

	rc = duk_del_prop_string(ctx, 0, "nonexistent");
	printf("delete obj.nonexistent -> rc=%d\n", (int) rc);

	rc = duk_del_prop_string(ctx, 0, "123");
	printf("delete obj['123'] -> rc=%d\n", (int) rc);

	rc = duk_del_prop_string(ctx, 1, "nonexistent");
	printf("delete arr.nonexistent -> rc=%d\n", (int) rc);

	rc = duk_del_prop_string(ctx, 1, "2");
	printf("delete arr['2'] -> rc=%d\n", (int) rc);

	duk_json_encode(ctx, 0);
	printf("final object: %s\n", duk_to_string(ctx, 0));
	duk_json_encode(ctx, 1);
	printf("final array: %s\n", duk_to_string(ctx, 1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_string(), non-configurable property (array 'length' property).
 * Same behavior when called inside/outside of a Duktape/C activation.
 */
static duk_ret_t test_2b(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_del_prop_string(ctx, 1, "length");
	printf("delete arr.length -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_string(), non-configurable virtual property of a plain string.
 * Same behavior when called inside/outside of a Duktape/C activation.
 */
static duk_ret_t test_2c(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_del_prop_string(ctx, 2, "5");
	printf("delete 'test_string'['5'] -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_string(), non-configurable virtual property of a plain string.
 * Same behavior when called inside/outside of a Duktape/C activation.
 */
static duk_ret_t test_2d(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_del_prop_string(ctx, 2, "length");
	printf("delete 'test_string'.length -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_string(), invalid index */
static duk_ret_t test_2e(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_del_prop_string(ctx, 234, "foo");
	printf("delete obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_string(), DUK_INVALID_INDEX */
static duk_ret_t test_2f(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_del_prop_string(ctx, DUK_INVALID_INDEX, "foo");
	printf("delete obj.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_string(), not object coercible */
static duk_ret_t test_2g(duk_context *ctx) {
	duk_ret_t rc;

	duk_set_top(ctx, 0);
	duk_push_null(ctx);
	rc = duk_del_prop_string(ctx, -1, "foo");
	printf("delete null.foo -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_index(), success cases */
static duk_ret_t test_3a(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_del_prop_index(ctx, 0, 31337);
	printf("delete obj[31337] -> rc=%d\n", (int) rc);

	rc = duk_del_prop_index(ctx, 0, 123);
	printf("delete obj[123] -> rc=%d\n", (int) rc);

	rc = duk_del_prop_index(ctx, 1, 31337);
	printf("delete arr[31337] -> rc=%d\n", (int) rc);

	rc = duk_del_prop_index(ctx, 1, 2);
	printf("delete arr[2] -> rc=%d\n", (int) rc);

	duk_json_encode(ctx, 0);
	printf("final object: %s\n", duk_to_string(ctx, 0));
	duk_json_encode(ctx, 1);
	printf("final array: %s\n", duk_to_string(ctx, 1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_index(), non-configurable virtual property of a plain string.
 * Same behavior when called inside/outside of a Duktape/C activation.
 */
static duk_ret_t test_3b(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_del_prop_index(ctx, 2, 5);
	printf("delete 'test_string'[5] -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_index(), invalid index */
static duk_ret_t test_3c(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_del_prop_index(ctx, 234, 123);
	printf("delete obj[123] -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_index(), DUK_INVALID_INDEX */
static duk_ret_t test_3d(duk_context *ctx) {
	duk_ret_t rc;

	prep(ctx);

	rc = duk_del_prop_index(ctx, DUK_INVALID_INDEX, 123);
	printf("delete obj[123] -> rc=%d\n", (int) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1a);
	TEST_SAFE_CALL(test_1b);
	TEST_PCALL(test_1b);
	TEST_SAFE_CALL(test_1c);
	TEST_PCALL(test_1c);
	TEST_SAFE_CALL(test_1d);
	TEST_PCALL(test_1d);
	TEST_SAFE_CALL(test_1e);
	TEST_PCALL(test_1e);
	TEST_SAFE_CALL(test_1f);
	TEST_PCALL(test_1f);
	TEST_SAFE_CALL(test_1g);
	TEST_PCALL(test_1g);

	TEST_SAFE_CALL(test_2a);
	TEST_SAFE_CALL(test_2b);
	TEST_PCALL(test_2b);
	TEST_SAFE_CALL(test_2c);
	TEST_PCALL(test_2c);
	TEST_SAFE_CALL(test_2d);
	TEST_PCALL(test_2d);
	TEST_SAFE_CALL(test_2e);
	TEST_PCALL(test_2e);
	TEST_SAFE_CALL(test_2f);
	TEST_PCALL(test_2f);
	TEST_SAFE_CALL(test_2g);
	TEST_PCALL(test_2g);

	TEST_SAFE_CALL(test_3a);
	TEST_SAFE_CALL(test_3b);
	TEST_PCALL(test_3b);
	TEST_SAFE_CALL(test_3c);
	TEST_PCALL(test_3c);
	TEST_SAFE_CALL(test_3d);
	TEST_PCALL(test_3d);
}
