/*===
*** test_range_error (duk_pcall)
==> rc=1
ToString(error): RangeError: range error: 123
name: RangeError
message: range error: 123
code: undefined
fileName is a string: 1
lineNumber: 63
isNative: undefined
*** test_arbitrary_code (duk_pcall)
==> rc=1
ToString(error): Error: arbitrary error code
name: Error
message: arbitrary error code
code: undefined
fileName is a string: 1
lineNumber: 74
isNative: undefined
*** test_null_message (duk_pcall)
==> rc=1
ToString(error): TypeError: 6
name: TypeError
message: 6
code: undefined
fileName is a string: 1
lineNumber: 86
isNative: undefined
*** test_vararg (duk_pcall)
==> rc=1
ToString(error): RangeError: my error 123 234 foobar
name: RangeError
message: my error 123 234 foobar
code: undefined
fileName is a string: 1
lineNumber: 97
isNative: undefined
*** test_error_return (duk_pcall)
==> rc=1
ToString(error): URIError: invalid argument uri
name: URIError
message: invalid argument uri
code: undefined
fileName is a string: 1
lineNumber: 124
isNative: undefined
*** test_error_va_return (duk_pcall)
==> rc=1
ToString(error): RangeError: my error 123 234 foobar
name: RangeError
message: my error 123 234 foobar
code: undefined
fileName is a string: 1
lineNumber: 104
isNative: undefined
===*/

static duk_ret_t test_range_error(duk_context *ctx, void *udata) {
	(void) udata;

	duk_set_top(ctx, 0);

	duk_error(ctx, DUK_ERR_RANGE_ERROR, "range error: %d", 123);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_arbitrary_code(duk_context *ctx, void *udata) {
	(void) udata;

	duk_set_top(ctx, 0);

	duk_error(ctx, 1234567, "arbitrary error code");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_null_message(duk_context *ctx, void *udata) {
	(void) udata;

	duk_set_top(ctx, 0);

	/* error code replaces message automatically now if message is NULL */
	duk_error(ctx, DUK_ERR_TYPE_ERROR, NULL);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Vararg */
static void my_error_1(duk_context *ctx, duk_errcode_t errcode, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	duk_error_va(ctx, errcode, fmt, ap);
	va_end(ap);
}
static duk_ret_t my_error_2(duk_context *ctx, duk_errcode_t errcode, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	return duk_error_va(ctx, errcode, fmt, ap);
	va_end(ap);
}

static duk_ret_t test_vararg(duk_context *ctx, void *udata) {
	(void) udata;

	duk_set_top(ctx, 0);

	my_error_1(ctx, DUK_ERR_RANGE_ERROR, "my error %d %d %s", 123, 234, "foobar");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_error_return(duk_context *ctx, void *udata) {
	(void) udata;

	duk_set_top(ctx, 0);

	return duk_error(ctx, DUK_ERR_URI_ERROR, "invalid argument uri");
}

static duk_ret_t test_error_va_return(duk_context *ctx, void *udata) {
	(void) udata;
	duk_set_top(ctx, 0);

	return my_error_2(ctx, DUK_ERR_RANGE_ERROR, "my error %d %d %s", 123, 234, "foobar");
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
		rc = duk_safe_call(ctx, (func), NULL, 0, 1); \
		printf("==> rc=%d\n", (int) rc); \
		dump_error(ctx); \
		duk_pop(ctx); \
	} while (0)

void test(duk_context *ctx) {
	duk_int_t rc;

	/* special test macro because of dump_error() */
	TEST(test_range_error);
	TEST(test_arbitrary_code);
	TEST(test_null_message);
	TEST(test_vararg);
	TEST(test_error_return);
	TEST(test_error_va_return);
}
