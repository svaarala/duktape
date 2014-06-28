/*===
*** test_1a (duk_safe_call)
push string with maximum size_t (should fail)
==> rc=1, result='RangeError: string too long'
*** test_1b (duk_safe_call)
push string with maximum size_t - 8 (should fail)
==> rc=1, result='RangeError: string too long'
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

const char dummy[4] = { 'f', 'o', 'o', '\0' };

static duk_ret_t test_1a(duk_context *ctx) {
	const char *p;

	duk_set_top(ctx, 0);

	printf("push string with maximum size_t (should fail)\n");
	p = duk_push_lstring(ctx, dummy, DUK_SIZE_MAX);
	printf("ptr is non-NULL: %d\n", (p != NULL ? 1 : 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_1b(duk_context *ctx) {
	const char *p;

	duk_set_top(ctx, 0);

	printf("push string with maximum size_t - 8 (should fail)\n");
	p = duk_push_lstring(ctx, dummy, DUK_SIZE_MAX - 8);
	printf("ptr is non-NULL: %d\n", (p != NULL ? 1 : 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	/*printf("DUK_SIZE_MAX=%lf\n", (double) DUK_SIZE_MAX);*/

	TEST_SAFE_CALL(test_1a);
	TEST_SAFE_CALL(test_1b);
}
