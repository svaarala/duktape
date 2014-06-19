/*===
*** test_1a
push string with maximum size_t (should fail)
rc=1, result='RangeError: string too long'
*** test_1b
push string with maximum size_t - 8 (should fail)
rc=1, result='RangeError: string too long'
===*/

/* Same as test-bug-push-buffer-maxsize.c but for string pushing.
 *
 * There were actually two bugs in the  implementation previously:
 * (1) the size computation for the header plus string may overflow,
 * and (2) the string size is passed as a duk_u32 internally which
 * clamps incorrectly on 64-bit platforms.
 *
 * The attempt to push a string of SIZE_MAX (or close) should fail
 * before the string data is actually read (there isn't enough data,
 * of course, if that were to happen).
 *
 * The fix, now implemented, is to check for string maximum size
 * explicitly.
 */

#ifndef  SIZE_MAX
#define  SIZE_MAX  ((size_t) -1)
#endif

const char dummy[4] = { 'f', 'o', 'o', '\0' };

int test_1a(duk_context *ctx) {
	const char *p;

	duk_set_top(ctx, 0);

	printf("push string with maximum size_t (should fail)\n");
	p = duk_push_lstring(ctx, dummy, SIZE_MAX);
	printf("ptr is non-NULL: %d\n", (p != NULL ? 1 : 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

int test_1b(duk_context *ctx) {
	const char *p;

	duk_set_top(ctx, 0);

	printf("push string with maximum size_t - 8 (should fail)\n");
	p = duk_push_lstring(ctx, dummy, SIZE_MAX - 8);
	printf("ptr is non-NULL: %d\n", (p != NULL ? 1 : 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}
#define  TEST(func)  do { \
		printf("*** %s\n", #func); \
		rc = duk_safe_call(ctx, (func), 0, 1); \
		printf("rc=%d, result='%s'\n", (int) rc, duk_to_string(ctx, -1)); \
		duk_pop(ctx); \
	} while (0)

void test(duk_context *ctx) {
	int rc;

	/*printf("SIZE_MAX=%lf\n", (double) SIZE_MAX);*/

	TEST(test_1a);
	TEST(test_1b);
}

