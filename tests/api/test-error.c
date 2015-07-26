/*===
*** test_1 (duk_pcall)
==> rc=1
ToString(error): RangeError: range error: 123
name: RangeError
message: range error: 123
code: undefined
fileName is a string: 1
lineNumber: 43
isNative: undefined
*** test_2 (duk_pcall)
==> rc=1
ToString(error): Error: arbitrary error code
name: Error
message: arbitrary error code
code: undefined
fileName is a string: 1
lineNumber: 52
isNative: undefined
*** test_3 (duk_pcall)
==> rc=1
ToString(error): TypeError: 105
name: TypeError
message: 105
code: undefined
fileName is a string: 1
lineNumber: 62
isNative: undefined
*** test_4 (duk_pcall)
==> rc=1
ToString(error): RangeError: my error 123 234 foobar
name: RangeError
message: my error 123 234 foobar
code: undefined
fileName is a string: 1
lineNumber: 73
isNative: undefined
===*/

static duk_ret_t test_1(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_error(ctx, DUK_ERR_RANGE_ERROR, "range error: %d", 123);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_2(duk_context *ctx) {
	duk_set_top(ctx, 0);

	duk_error(ctx, 1234567, "arbitrary error code");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_3(duk_context *ctx) {
	duk_set_top(ctx, 0);

	/* error code replaces message automatically now if message is NULL */
	duk_error(ctx, DUK_ERR_TYPE_ERROR, NULL);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Vararg */
static void my_error(duk_context *ctx, duk_errcode_t errcode, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	duk_error_va(ctx, errcode, fmt, ap);
	va_end(ap);
}

static duk_ret_t test_4(duk_context *ctx) {
	duk_set_top(ctx, 0);

	my_error(ctx, DUK_ERR_RANGE_ERROR, "my error %d %d %s", 123, 234, "foobar");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
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

	/* 'code' is no longer set, test that it reads back as 'undefined' */
	duk_get_prop_string(ctx, -1, "code");
	printf("code: %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "fileName");
	printf("fileName is a string: %d\n", (int) duk_is_string(ctx, -1));
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "lineNumber");
	printf("lineNumber: %ld\n", (long) duk_get_int(ctx, -1));
	duk_pop(ctx);

	/* 'isNative' has also been removed, check that it reads back as 'undefined' */
	duk_get_prop_string(ctx, -1, "isNative");
	printf("isNative: %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);
}

/* use custom helper because of dump_error() */
#define  TEST(func)  do {  \
		printf("*** %s (duk_pcall)\n", #func); \
		rc = duk_safe_call(ctx, (func), 0, 1); \
		printf("==> rc=%d\n", (int) rc); \
		dump_error(ctx); \
		duk_pop(ctx); \
	} while (0)

void test(duk_context *ctx) {
	duk_int_t rc;

	/* special test macro because of dump_error() */
	TEST(test_1);
	TEST(test_2);
	TEST(test_3);
	TEST(test_4);
}
