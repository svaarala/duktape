/*===
*** test_1 (duk_pcall)
==> rc=1
ToString(error): RangeError: range error: 123
name: RangeError
message: range error: 123
code: 102
fileName: undefined
lineNumber: 0
isNative: undefined
*** test_2 (duk_pcall)
==> rc=1
ToString(error): Error: arbitrary error code
name: Error
message: arbitrary error code
code: 1234567
fileName: undefined
lineNumber: 0
isNative: undefined
*** test_3 (duk_pcall)
==> rc=1
ToString(error): TypeError
name: TypeError
message: 
code: 105
fileName: undefined
lineNumber: 0
isNative: undefined
===*/

/* FIXME: fileName, lineNumber, isNative are currently not set. */

int test_1(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_error(ctx, DUK_ERR_RANGE_ERROR, "range error: %d", 123);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

int test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_error(ctx, 1234567, "arbitrary error code");

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

int test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_error(ctx, DUK_ERR_TYPE_ERROR, NULL);

	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

void dump_error(duk_context *ctx) {
	duk_dup(ctx, -1);
	printf("ToString(error): %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "name");
	printf("name: %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "message");
	printf("message: %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "code");
	printf("code: %d\n", duk_get_int(ctx, -1));
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "fileName");
	printf("fileName: %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "lineNumber");
	printf("lineNumber: %d\n", duk_get_int(ctx, -1));
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "isNative");
	printf("isNative: %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);
}

/* use custom helper because of dump_error() */
#define  TEST(func)  do {  \
		printf("*** %s (duk_pcall)\n", #func); \
		rc = duk_safe_call(ctx, (func), 0, 1, DUK_INVALID_INDEX); \
		printf("==> rc=%d\n", rc); \
		dump_error(ctx); \
		duk_pop(ctx); \
	} while (0)

void test(duk_context *ctx) {
	int rc;

	TEST(test_1);
	TEST(test_2);
	TEST(test_3);
}

