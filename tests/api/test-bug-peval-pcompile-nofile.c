/*
 *  In Duktape 0.12.0 duk_peval_file() and variants would throw an error if
 *  a file did not exist.  The macros implementing these variants use
 *  duk_push_string_file() which throws an error for a non-existent file.
 *
 *  Reported by David Demelier.
 */

/*---
{
    "skip": true
}
---*/

#define  NONEXISTENT_FILE  "/tmp/this/file/does/not/exist"

/*===
*** test_peval_file (duk_safe_call)
rc: 1
result: Error: no sourcecode
final top: 1
==> rc=0, result='undefined'
*** test_peval_file_noresult (duk_safe_call)
rc: 1
final top: 0
==> rc=0, result='undefined'
*** test_pcompile_file (duk_safe_call)
rc: 1
result: Error: no sourcecode
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_peval_file(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	rc = duk_peval_file(ctx, NONEXISTENT_FILE);
	printf("rc: %ld\n", (long) rc);
	printf("result: %s\n", duk_safe_to_string(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_peval_file_noresult(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	rc = duk_peval_file_noresult(ctx, NONEXISTENT_FILE);
	printf("rc: %ld\n", (long) rc);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_pcompile_file(duk_context *ctx, void *udata) {
	duk_int_t rc;

	(void) udata;

	rc = duk_pcompile_file(ctx, 0, NONEXISTENT_FILE);
	printf("rc: %ld\n", (long) rc);
	printf("result: %s\n", duk_safe_to_string(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_peval_file);
	TEST_SAFE_CALL(test_peval_file_noresult);
	TEST_SAFE_CALL(test_pcompile_file);
}
