/*===
*** test_1a
delete obj.foo -> rc=1
delete obj.nonexistent -> rc=1
delete obj[123] -> rc=1
delete arr.nonexistent -> rc=1
delete arr[2] -> rc=1
delete arr.length -> rc=0
delete 'test_string'['5'] -> rc=0
delete 'test_string'.length -> rc=0
final object: {"bar":"barval"}
final array: ["foo","bar",null]
final top: 7
rc=0, result='undefined'
*** test_1b (wrapped)
rc=1, result='TypeError: property not configurable'
*** test_1c (wrapped)
rc=1, result='TypeError: property not configurable'
*** test_1d
rc=1, result='Error: index out of bounds'
*** test_1e
rc=1, result='Error: index out of bounds'
*** test_1f
rc=1, result='TypeError: invalid base reference for property delete'
*** test_2a
delete obj.foo -> rc=1
delete obj.nonexistent -> rc=1
delete obj['123'] -> rc=1
delete arr.nonexistent -> rc=1
delete arr['2'] -> rc=1
delete arr.length -> rc=0
delete 'test_string'['5'] -> rc=0
delete 'test_string'.length -> rc=0
final object: {"bar":"barval"}
final array: ["foo","bar",null]
final top: 7
rc=0, result='undefined'
*** test_2b (wrapped)
rc=1, result='TypeError: property not configurable'
*** test_2c (wrapped)
rc=1, result='TypeError: property not configurable'
*** test_2d
rc=1, result='Error: invalid index: 234'
*** test_2e
rc=1, result='Error: invalid index: -2147483648'
*** test_3a
delete obj[31337] -> rc=1
delete obj[123] -> rc=1
delete arr[31337] -> rc=1
delete arr[2] -> rc=1
delete 'test_string'[5] -> rc=0
final object: {"foo":"fooval","bar":"barval"}
final array: ["foo","bar",null]
final top: 5
rc=0, result='undefined'
*** test_3b (wrapped)
rc=1, result='TypeError: property not configurable'
*** test_3c
rc=1, result='Error: invalid index: 234'
*** test_3d
rc=1, result='Error: invalid index: -2147483648'
===*/

void prep(duk_context *ctx) {
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
int test_1a(duk_context *ctx) {
	int rc;

	prep(ctx);

	/* existing, configurable */
	duk_push_string(ctx, "foo");
	rc = duk_del_prop(ctx, 0);
	printf("delete obj.foo -> rc=%d\n", rc);

	/* nonexistent */
	duk_push_string(ctx, "nonexistent");
	rc = duk_del_prop(ctx, 0);
	printf("delete obj.nonexistent -> rc=%d\n", rc);

 	/* nonexistent */
	duk_push_int(ctx, 123);
	rc = duk_del_prop(ctx, 0);
	printf("delete obj[123] -> rc=%d\n", rc);

	/* nonexistent, array */
	duk_push_string(ctx, "nonexistent");
	rc = duk_del_prop(ctx, 1);
	printf("delete arr.nonexistent -> rc=%d\n", rc);

	/* existing, configurable, array */
	duk_push_int(ctx, 2);
	rc = duk_del_prop(ctx, 1);
	printf("delete arr[2] -> rc=%d\n", rc);

	/* non-configurable property, but called from outside a Duktape/C
	 * activation, so obeys non-strict semantics.
	 */
	duk_push_string(ctx, "length");
	rc = duk_del_prop(ctx, 1);
	printf("delete arr.length -> rc=%d\n", rc);

	/* non-configurable virtual property of a string, non-strict mode.
	 */
	duk_push_int(ctx, 5);
	rc = duk_del_prop(ctx, 2);
	printf("delete 'test_string'['5'] -> rc=%d\n", rc);

	/* non-configurable virtual property of a string, non-strict mode.
	 */
	duk_push_string(ctx, "length");
	rc = duk_del_prop(ctx, 2);
	printf("delete 'test_string'.length -> rc=%d\n", rc);

	duk_json_encode(ctx, 0);
	printf("final object: %s\n", duk_to_string(ctx, 0));
	duk_json_encode(ctx, 1);
	printf("final array: %s\n", duk_to_string(ctx, 1));

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* duk_del_prop(), non-configurable property (array 'length' property),
 * called from inside a Duktape/C context.
 */
int test_1b(duk_context *ctx) {
	int rc;

	prep(ctx);

	duk_push_string(ctx, "length");
	rc = duk_del_prop(ctx, 1);
	printf("delete arr.length -> rc=%d\n", rc);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* duk_del_prop(), non-configurable virtual property of a plain string,
 * called from inside a Duktape/C context.
*/
int test_1c(duk_context *ctx) {
	int rc;

	prep(ctx);

	duk_push_int(ctx, 5);
	rc = duk_del_prop(ctx, 2);
	printf("delete 'test_string'[5] -> rc=%d\n", rc);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* duk_del_prop(), invalid index */
int test_1d(duk_context *ctx) {
	int rc;

	prep(ctx);

	duk_push_string(ctx, "foo");
	rc = duk_del_prop(ctx, 234);
	printf("delete obj.foo -> rc=%d\n", rc);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* duk_del_prop(), DUK_INVALID_INDEX */
int test_1e(duk_context *ctx) {
	int rc;

	prep(ctx);

	duk_push_string(ctx, "foo");
	rc = duk_del_prop(ctx, DUK_INVALID_INDEX);
	printf("delete obj.foo -> rc=%d\n", rc);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* duk_del_prop(), not object coercible */
int test_1f(duk_context *ctx) {
	int rc;

	duk_set_top(ctx, 0);
	duk_push_null(ctx);
	duk_push_string(ctx, "foo");
	rc = duk_del_prop(ctx, -2);
	printf("delete null.foo -> rc=%d\n", rc);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_string(), success cases */
int test_2a(duk_context *ctx) {
	int rc;

	prep(ctx);

	rc = duk_del_prop_string(ctx, 0, "foo");
	printf("delete obj.foo -> rc=%d\n", rc);

	rc = duk_del_prop_string(ctx, 0, "nonexistent");
	printf("delete obj.nonexistent -> rc=%d\n", rc);

	rc = duk_del_prop_string(ctx, 0, "123");
	printf("delete obj['123'] -> rc=%d\n", rc);

	rc = duk_del_prop_string(ctx, 1, "nonexistent");
	printf("delete arr.nonexistent -> rc=%d\n", rc);

	rc = duk_del_prop_string(ctx, 1, "2");
	printf("delete arr['2'] -> rc=%d\n", rc);

	/* non-configurable property, but running in non-strict mode */
	rc = duk_del_prop_string(ctx, 1, "length");
	printf("delete arr.length -> rc=%d\n", rc);

	/* non-configurable property, but running in non-strict mode */
	rc = duk_del_prop_string(ctx, 2, "5");
	printf("delete 'test_string'['5'] -> rc=%d\n", rc);

	rc = duk_del_prop_string(ctx, 2, "length");
	printf("delete 'test_string'.length -> rc=%d\n", rc);

	duk_json_encode(ctx, 0);
	printf("final object: %s\n", duk_to_string(ctx, 0));
	duk_json_encode(ctx, 1);
	printf("final array: %s\n", duk_to_string(ctx, 1));

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_string(), non-configurable property (array 'length' property),
 * called from inside a Duktape/C context.
 */
int test_2b(duk_context *ctx) {
	int rc;

	prep(ctx);

	rc = duk_del_prop_string(ctx, 1, "length");
	printf("delete arr.length -> rc=%d\n", rc);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_string(), non-configurable virtual property of a plain string,
 * called from inside a Duktape/C context.
*/
int test_2c(duk_context *ctx) {
	int rc;

	prep(ctx);

	rc = duk_del_prop_string(ctx, 2, "5");
	printf("delete 'test_string'['5'] -> rc=%d\n", rc);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_string(), invalid index */
int test_2d(duk_context *ctx) {
	int rc;

	prep(ctx);

	rc = duk_del_prop_string(ctx, 234, "foo");
	printf("delete obj.foo -> rc=%d\n", rc);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_string(), DUK_INVALID_INDEX */
int test_2e(duk_context *ctx) {
	int rc;

	prep(ctx);

	rc = duk_del_prop_string(ctx, DUK_INVALID_INDEX, "foo");
	printf("delete obj.foo -> rc=%d\n", rc);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_index(), success cases */
int test_3a(duk_context *ctx) {
	int rc;

	prep(ctx);

	rc = duk_del_prop_index(ctx, 0, 31337);
	printf("delete obj[31337] -> rc=%d\n", rc);

	rc = duk_del_prop_index(ctx, 0, 123);
	printf("delete obj[123] -> rc=%d\n", rc);

	rc = duk_del_prop_index(ctx, 1, 31337);
	printf("delete arr[31337] -> rc=%d\n", rc);

	rc = duk_del_prop_index(ctx, 1, 2);
	printf("delete arr[2] -> rc=%d\n", rc);

	rc = duk_del_prop_index(ctx, 2, 5);
	printf("delete 'test_string'[5] -> rc=%d\n", rc);

	duk_json_encode(ctx, 0);
	printf("final object: %s\n", duk_to_string(ctx, 0));
	duk_json_encode(ctx, 1);
	printf("final array: %s\n", duk_to_string(ctx, 1));

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_index(), non-configurable virtual property of a plain string,
 * called from inside a Duktape/C context.
*/
int test_3b(duk_context *ctx) {
	int rc;

	prep(ctx);

	rc = duk_del_prop_index(ctx, 2, 5);
	printf("delete 'test_string'[5] -> rc=%d\n", rc);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_index(), invalid index */
int test_3c(duk_context *ctx) {
	int rc;

	prep(ctx);

	rc = duk_del_prop_index(ctx, 234, 123);
	printf("delete obj[123] -> rc=%d\n", rc);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* duk_del_prop_index(), DUK_INVALID_INDEX */
int test_3d(duk_context *ctx) {
	int rc;

	prep(ctx);

	rc = duk_del_prop_index(ctx, DUK_INVALID_INDEX, 123);
	printf("delete obj[123] -> rc=%d\n", rc);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

/* execute test outside of a Duktape/C activation (= non-strict mode) */
#define  TEST(func)  do { \
		printf("*** %s\n", #func); \
		rc = duk_safe_call(ctx, (func), 0, 1, DUK_INVALID_INDEX); \
		printf("rc=%d, result='%s'\n", rc, duk_to_string(ctx, -1)); \
		duk_pop(ctx); \
	} while(0)

/* execute test inside of a Duktape/C activation (= strict mode) */
#define  TESTWRAPPED(func)  do { \
		printf("*** %s (wrapped)\n", #func); \
		duk_push_new_c_function(ctx, (func), 0); \
		rc = duk_pcall(ctx, 0, DUK_INVALID_INDEX); \
		printf("rc=%d, result='%s'\n", rc, duk_to_string(ctx, -1)); \
		duk_pop(ctx); \
	} while(0)

void test(duk_context *ctx) {
	int rc;

	TEST(test_1a);
	TESTWRAPPED(test_1b);
	TESTWRAPPED(test_1c);
	TEST(test_1d);
	TEST(test_1e);
	TEST(test_1f);

	TEST(test_2a);
	TESTWRAPPED(test_2b);
	TESTWRAPPED(test_2c);
	TEST(test_2d);
	/* FIXME: currently error message contains the actual DUK_INVALID_INDEX
	 * value, nonportable */
	TEST(test_2e);

	TEST(test_3a);
	TESTWRAPPED(test_3b);
	TEST(test_3c);
	/* FIXME: currently error message contains the actual DUK_INVALID_INDEX
	 * value, nonportable */
	TEST(test_3d);
}

