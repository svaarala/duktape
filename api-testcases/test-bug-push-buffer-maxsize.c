/*===
*** test_1a
fixed size buffer, maximum size_t (should fail)
rc=1, result='RangeError: buffer too long'
*** test_1b
fixed size buffer, maximum size_t - 8 (should fail)
rc=1, result='RangeError: buffer too long'
*** test_2a
dynamic size buffer, maximum size_t (should fail)
rc=1, result='RangeError: buffer too long'
*** test_2b
dynamic size buffer, maximum size_t - 8 (should fail)
rc=1, result='RangeError: buffer too long'
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

#ifndef  SIZE_MAX
#define  SIZE_MAX  ((size_t) -1)
#endif

int test_1a(duk_context *ctx) {
	void *buf;

	duk_set_top(ctx, 0);

	printf("fixed size buffer, maximum size_t (should fail)\n");
	buf = duk_push_buffer(ctx, SIZE_MAX, 0);
	printf("ptr is non-NULL: %d\n", (buf != NULL ? 1 : 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

int test_1b(duk_context *ctx) {
	void *buf;

	duk_set_top(ctx, 0);

	printf("fixed size buffer, maximum size_t - 8 (should fail)\n");
	buf = duk_push_buffer(ctx, SIZE_MAX - 8, 0);
	printf("ptr is non-NULL: %d\n", (buf != NULL ? 1 : 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

int test_2a(duk_context *ctx) {
	void *buf;

	duk_set_top(ctx, 0);

	printf("dynamic size buffer, maximum size_t (should fail)\n");
	buf = duk_push_buffer(ctx, SIZE_MAX, 1);
	printf("ptr is non-NULL: %d\n", (buf != NULL ? 1 : 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* This test actually succeeds even with the bug. */
int test_2b(duk_context *ctx) {
	void *buf;

	duk_set_top(ctx, 0);

	printf("dynamic size buffer, maximum size_t - 8 (should fail)\n");
	buf = duk_push_buffer(ctx, SIZE_MAX - 8, 1);
	printf("ptr is non-NULL: %d\n", (buf != NULL ? 1 : 0));

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
	TEST(test_2a);
	TEST(test_2b);
}
