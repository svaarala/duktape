/*===
*** test_1a (duk_safe_call)
fixed size buffer, maximum size_t (should fail)
==> rc=1, result='RangeError: buffer too long'
*** test_1b (duk_safe_call)
fixed size buffer, maximum size_t - 8 (should fail)
==> rc=1, result='RangeError: buffer too long'
*** test_2a (duk_safe_call)
dynamic size buffer, maximum size_t (should fail)
==> rc=1, result='RangeError: buffer too long'
*** test_2b (duk_safe_call)
dynamic size buffer, maximum size_t - 8 (should fail)
==> rc=1, result='RangeError: buffer too long'
===*/

/* Before Duktape 0.9.0, an attempt to allocate a buffer of maximum size_t
 * (or anything so close that when the heap header size is added, the result
 * overflows) causes a spurious successful allocation now.  The allocation
 * will in fact be too little to even contain the heap header but will appear
 * to succeed.
 *
 * The proper behavior is to check for a maximum size before adding the header
 * size to the requested size (this is done now).
 */

static int test_1a(duk_context *ctx) {
	void *buf;

	duk_set_top(ctx, 0);

	printf("fixed size buffer, maximum size_t (should fail)\n");
	buf = duk_push_buffer(ctx, DUK_SIZE_MAX, 0);
	printf("ptr is non-NULL: %d\n", (buf != NULL ? 1 : 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static int test_1b(duk_context *ctx) {
	void *buf;

	duk_set_top(ctx, 0);

	printf("fixed size buffer, maximum size_t - 8 (should fail)\n");
	buf = duk_push_buffer(ctx, DUK_SIZE_MAX - 8, 0);
	printf("ptr is non-NULL: %d\n", (buf != NULL ? 1 : 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static int test_2a(duk_context *ctx) {
	void *buf;

	duk_set_top(ctx, 0);

	printf("dynamic size buffer, maximum size_t (should fail)\n");
	buf = duk_push_buffer(ctx, DUK_SIZE_MAX, 1);
	printf("ptr is non-NULL: %d\n", (buf != NULL ? 1 : 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* This test actually succeeds even with the bug. */
static int test_2b(duk_context *ctx) {
	void *buf;

	duk_set_top(ctx, 0);

	printf("dynamic size buffer, maximum size_t - 8 (should fail)\n");
	buf = duk_push_buffer(ctx, DUK_SIZE_MAX - 8, 1);
	printf("ptr is non-NULL: %d\n", (buf != NULL ? 1 : 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	/*printf("DUK_SIZE_MAX=%lf\n", (double) DUK_SIZE_MAX);*/

	TEST_SAFE_CALL(test_1a);
	TEST_SAFE_CALL(test_1b);
	TEST_SAFE_CALL(test_2a);
	TEST_SAFE_CALL(test_2b);
}
