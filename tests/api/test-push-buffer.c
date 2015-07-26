/*===
*** test_1a (duk_safe_call)
fixed size, 0 bytes (no guarantee whether ptr NULL or non-NULL)
buffer should be all zeroes
buffer should be writable
fixed size, 1024 bytes
ptr is non-NULL: 1
buffer should be all zeroes
buffer should be writable
dynamic size, 0 bytes (no guarantee whether ptr NULL or non-NULL)
buffer should be all zeroes
buffer should be writable
dynamic size, 1024 bytes
ptr is non-NULL: 1
buffer should be all zeroes
buffer should be writable
final top: 4
==> rc=0, result='undefined'
*** test_1b (duk_safe_call)
fixed size, 0 bytes (no guarantee whether ptr NULL or non-NULL)
buffer should be all zeroes
buffer should be writable
fixed size, 1024 bytes
ptr is non-NULL: 1
buffer should be all zeroes
buffer should be writable
dynamic size, 0 bytes (no guarantee whether ptr NULL or non-NULL)
buffer should be all zeroes
buffer should be writable
dynamic size, 1024 bytes
ptr is non-NULL: 1
buffer should be all zeroes
buffer should be writable
final top: 4
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
fixed size buffer, close to maximum size_t (should fail)
==> rc=1, result='RangeError: buffer too long'
*** test_3 (duk_safe_call)
dynamic size buffer, close to maximum size_t (should fail)
==> rc=1, result='RangeError: buffer too long'
===*/

#ifndef  SIZE_MAX
#define  SIZE_MAX  ((size_t) -1)
#endif

static void rw_test(unsigned char *p, size_t sz) {
	int i;

	printf("buffer should be all zeroes\n");
	for (i = 0; i < sz; i++) {
		if (p[i] != 0) {
			printf("buffer non-zero at index %d: 0x%02x\n", i, (int) p[i]);
		}
	}

	printf("buffer should be writable\n");
	for (i = 0; i < sz; i++) {
		p[i] = 0xff;
	}
}

/* Basic success test. */
static duk_ret_t test_1a(duk_context *ctx) {
	void *buf;

	duk_set_top(ctx, 0);

	printf("fixed size, 0 bytes (no guarantee whether ptr NULL or non-NULL)\n");
	buf = duk_push_buffer(ctx, 0, 0);
	rw_test((unsigned char *) buf, 0);

	printf("fixed size, 1024 bytes\n");
	buf = duk_push_buffer(ctx, 1024, 0);
	printf("ptr is non-NULL: %d\n", (buf != NULL ? 1 : 0));
	rw_test((unsigned char *) buf, 1024);

	printf("dynamic size, 0 bytes (no guarantee whether ptr NULL or non-NULL)\n");
	buf = duk_push_buffer(ctx, 0, 1);
	rw_test((unsigned char *) buf, 0);

	printf("dynamic size, 1024 bytes\n");
	buf = duk_push_buffer(ctx, 1024, 1);
	printf("ptr is non-NULL: %d\n", (buf != NULL ? 1 : 0));
	rw_test((unsigned char *) buf, 1024);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Same test, using shortcut functions. */
static duk_ret_t test_1b(duk_context *ctx) {
	void *buf;

	duk_set_top(ctx, 0);

	printf("fixed size, 0 bytes (no guarantee whether ptr NULL or non-NULL)\n");
	buf = duk_push_fixed_buffer(ctx, 0);
	rw_test((unsigned char *) buf, 0);

	printf("fixed size, 1024 bytes\n");
	buf = duk_push_fixed_buffer(ctx, 1024);
	printf("ptr is non-NULL: %d\n", (buf != NULL ? 1 : 0));
	rw_test((unsigned char *) buf, 1024);

	printf("dynamic size, 0 bytes (no guarantee whether ptr NULL or non-NULL)\n");
	buf = duk_push_dynamic_buffer(ctx, 0);
	rw_test((unsigned char *) buf, 0);

	printf("dynamic size, 1024 bytes\n");
	buf = duk_push_dynamic_buffer(ctx, 1024);
	printf("ptr is non-NULL: %d\n", (buf != NULL ? 1 : 0));
	rw_test((unsigned char *) buf, 1024);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}


/* Ensure that insanely large alloation fails.  Don't use SIZE_MAX, as
 * a current bug makes the allocated size wrap and actually causes
 * other trouble (separate bug testcase exists for that:
 * test-bug-push-buffer-maxsize.c).
 */
static duk_ret_t test_2(duk_context *ctx) {
	void *buf;

	duk_set_top(ctx, 0);

	printf("fixed size buffer, close to maximum size_t (should fail)\n");
	buf = duk_push_buffer(ctx, SIZE_MAX - 1024, 0);
	printf("ptr is non-NULL: %d\n", (buf != NULL ? 1 : 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_3(duk_context *ctx) {
	void *buf;

	duk_set_top(ctx, 0);

	printf("dynamic size buffer, close to maximum size_t (should fail)\n");
	buf = duk_push_buffer(ctx, SIZE_MAX - 1024, 1);
	printf("ptr is non-NULL: %d\n", (buf != NULL ? 1 : 0));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	/*printf("DUK_SIZE_MAX=%lf\n", (double) DUK_SIZE_MAX);*/

	TEST_SAFE_CALL(test_1a);
	TEST_SAFE_CALL(test_1b);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
}
