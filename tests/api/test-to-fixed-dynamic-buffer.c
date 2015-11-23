/*===
*** test_1a (duk_safe_call)
16 bytes (dynamic): 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2
q is NULL: 0
p == q: 0
sz=16
16 bytes (fixed): 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2
r is NULL: 0
q == r: 1
16 bytes (fixed): 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2
final top: 1
==> rc=0, result='undefined'
*** test_1b (duk_safe_call)
16 bytes (fixed): 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2
q is NULL: 0
p == q: 0
sz=16
16 bytes (dynamic): 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2
r is NULL: 0
q == r: 1
16 bytes (dynamic): 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2
final top: 1
==> rc=0, result='undefined'
*** test_2a (duk_safe_call)
16 bytes (fixed): 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2
q is NULL: 0
p == q: 1
sz=16
16 bytes (fixed): 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2
final top: 1
==> rc=0, result='undefined'
*** test_2b (duk_safe_call)
16 bytes (dynamic): 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2
q is NULL: 0
p == q: 1
sz=16
16 bytes (dynamic): 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2
final top: 1
==> rc=0, result='undefined'
*** test_3a (duk_safe_call)
q is NULL: 0
sz=3
3 bytes (fixed): 102 111 111
final top: 1
==> rc=0, result='undefined'
*** test_3b (duk_safe_call)
q is NULL: 0
sz=3
3 bytes (dynamic): 102 111 111
final top: 1
==> rc=0, result='undefined'
*** test_4a (duk_safe_call)
==> rc=1, result='Error: invalid stack index 3'
*** test_4b (duk_safe_call)
==> rc=1, result='Error: invalid stack index 3'
*** test_5a (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
*** test_5b (duk_safe_call)
==> rc=1, result='Error: invalid stack index -2147483648'
*** test_6a (duk_safe_call)
sz=16
p[0]=123, buf[0]=0
final top: 0
==> rc=0, result='undefined'
*** test_6b (duk_safe_call)
sz=16
p[0]=123, buf[0]=0
final top: 0
==> rc=0, result='undefined'
*** test_6c (duk_safe_call)
sz=16
p[0]=123, buf[0]=123
final top: 0
==> rc=0, result='undefined'
===*/

static void dump_buffer(duk_context *ctx) {
	unsigned char *p;
	duk_size_t i, sz;

	p = (unsigned char *) duk_require_buffer(ctx, -1, &sz);
	printf("%lu bytes (%s):", (unsigned long) sz,
	       (int) duk_is_dynamic_buffer(ctx, -1) ? "dynamic" : "fixed");
	for (i = 0; i < sz; i++) {
		printf(" %d", (int) p[i]);
	}
	printf("\n");
}

/* source: dynamic buffer, target: fixed buffer */
static duk_ret_t test_1a(duk_context *ctx) {
	unsigned char *p;
	void *q, *r;
	duk_size_t sz;

	duk_set_top(ctx, 0);

	p = (unsigned char *) duk_push_dynamic_buffer(ctx, 16);
	p[0] = 1;
	p[15] = 2;
	dump_buffer(ctx);

	sz = (duk_size_t) 1234;
	q = duk_to_fixed_buffer(ctx, -1, &sz);
	printf("q is NULL: %d\n", (q == NULL ? 1 : 0));
	printf("p == q: %d\n", (p == q ? 1 : 0));
	printf("sz=%lu\n", (unsigned long) sz);
	dump_buffer(ctx);

	/* second time should be a no-op */
	r = duk_to_fixed_buffer(ctx, -1, NULL);
	printf("r is NULL: %d\n", (q == NULL ? 1 : 0));
	printf("q == r: %d\n", (q == r ? 1 : 0));
	dump_buffer(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* source: fixed buffer, target: dynamic buffer */
static duk_ret_t test_1b(duk_context *ctx) {
	unsigned char *p;
	void *q, *r;
	duk_size_t sz;

	duk_set_top(ctx, 0);

	p = (unsigned char *) duk_push_fixed_buffer(ctx, 16);
	p[0] = 1;
	p[15] = 2;
	dump_buffer(ctx);

	sz = (duk_size_t) 1234;
	q = duk_to_dynamic_buffer(ctx, -1, &sz);
	printf("q is NULL: %d\n", (q == NULL ? 1 : 0));
	printf("p == q: %d\n", (p == q ? 1 : 0));
	printf("sz=%lu\n", (unsigned long) sz);
	dump_buffer(ctx);

	/* second time should be a no-op */
	r = duk_to_dynamic_buffer(ctx, -1, NULL);
	printf("r is NULL: %d\n", (q == NULL ? 1 : 0));
	printf("q == r: %d\n", (q == r ? 1 : 0));
	dump_buffer(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* source: fixed buffer, target: fixed buffer */
static duk_ret_t test_2a(duk_context *ctx) {
	unsigned char *p;
	void *q;
	duk_size_t sz;

	duk_set_top(ctx, 0);

	p = (unsigned char *) duk_push_fixed_buffer(ctx, 16);
	p[0] = 1;
	p[15] = 2;
	dump_buffer(ctx);

	q = duk_to_fixed_buffer(ctx, -1, &sz);
	printf("q is NULL: %d\n", (q == NULL ? 1 : 0));
	printf("p == q: %d\n", (p == q ? 1 : 0));
	printf("sz=%lu\n", (unsigned long) sz);
	dump_buffer(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* source: dynamic buffer, target: dynamic buffer */
static duk_ret_t test_2b(duk_context *ctx) {
	unsigned char *p;
	void *q;
	duk_size_t sz;

	duk_set_top(ctx, 0);

	p = (unsigned char *) duk_push_dynamic_buffer(ctx, 16);
	p[0] = 1;
	p[15] = 2;
	dump_buffer(ctx);

	q = duk_to_dynamic_buffer(ctx, -1, &sz);
	printf("q is NULL: %d\n", (q == NULL ? 1 : 0));
	printf("p == q: %d\n", (p == q ? 1 : 0));
	printf("sz=%lu\n", (unsigned long) sz);
	dump_buffer(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* source: non-buffer, target: fixed buffer */
static duk_ret_t test_3a(duk_context *ctx) {
	void *q;
	duk_size_t sz;

	duk_set_top(ctx, 0);
	duk_push_string(ctx, "foo");

	q = duk_to_fixed_buffer(ctx, -1, &sz);
	printf("q is NULL: %d\n", (q == NULL ? 1 : 0));
	printf("sz=%lu\n", (unsigned long) sz);
	dump_buffer(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* source: non-buffer, target: dynamic buffer */
static duk_ret_t test_3b(duk_context *ctx) {
	void *q;
	duk_size_t sz;

	duk_set_top(ctx, 0);
	duk_push_string(ctx, "foo");

	q = duk_to_dynamic_buffer(ctx, -1, &sz);
	printf("q is NULL: %d\n", (q == NULL ? 1 : 0));
	printf("sz=%lu\n", (unsigned long) sz);
	dump_buffer(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* invalid index, target: fixed buffer */
static duk_ret_t test_4a(duk_context *ctx) {
	void *q;
	duk_size_t sz;

	duk_set_top(ctx, 0);

	q = duk_to_fixed_buffer(ctx, 3, &sz);
	printf("q is NULL: %d\n", (q == NULL ? 1 : 0));
	printf("sz=%lu\n", (unsigned long) sz);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* invalid index, target: dynamic buffer */
static duk_ret_t test_4b(duk_context *ctx) {
	void *q;
	duk_size_t sz;

	duk_set_top(ctx, 0);

	q = duk_to_dynamic_buffer(ctx, 3, &sz);
	printf("q is NULL: %d\n", (q == NULL ? 1 : 0));
	printf("sz=%lu\n", (unsigned long) sz);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* DUK_INVALID_INDEX, target: fixed buffer */
static duk_ret_t test_5a(duk_context *ctx) {
	void *q;
	duk_size_t sz;

	duk_set_top(ctx, 0);

	q = duk_to_fixed_buffer(ctx, DUK_INVALID_INDEX, &sz);
	printf("q is NULL: %d\n", (q == NULL ? 1 : 0));
	printf("sz=%lu\n", (unsigned long) sz);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* DUK_INVALID_INDEX, target: dynamic buffer */
static duk_ret_t test_5b(duk_context *ctx) {
	void *q;
	duk_size_t sz;

	duk_set_top(ctx, 0);

	q = duk_to_dynamic_buffer(ctx, DUK_INVALID_INDEX, &sz);
	printf("q is NULL: %d\n", (q == NULL ? 1 : 0));
	printf("sz=%lu\n", (unsigned long) sz);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* When converting from an external buffer to a fixed buffer a copy is
 * always made.
 */
static duk_ret_t test_6a(duk_context *ctx) {
	unsigned char buf[16];
	int i;
	unsigned char *p;
	duk_size_t sz;

	for (i = 0; i < 16; i++) {
		buf[i] = (unsigned char) i;
	}

	duk_push_external_buffer(ctx);
	duk_config_buffer(ctx, -1, (void *) buf, 16);

	p = (unsigned char *) duk_to_fixed_buffer(ctx, -1, &sz);
	printf("sz=%ld\n", (long) sz); fflush(stdout);
	p[0] = (unsigned char) 123;
	printf("p[0]=%u, buf[0]=%u\n", (unsigned int) p[0], (unsigned int) buf[0]);

	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* When converting from an external buffer to a dynamic buffer a copy
 * is always made.
 */
static duk_ret_t test_6b(duk_context *ctx) {
	unsigned char buf[16];
	int i;
	unsigned char *p;
	duk_size_t sz;

	for (i = 0; i < 16; i++) {
		buf[i] = (unsigned char) i;
	}

	duk_push_external_buffer(ctx);
	duk_config_buffer(ctx, -1, (void *) buf, 16);

	p = (unsigned char *) duk_to_dynamic_buffer(ctx, -1, &sz);
	printf("sz=%ld\n", (long) sz); fflush(stdout);
	p[0] = (unsigned char) 123;
	printf("p[0]=%u, buf[0]=%u\n", (unsigned int) p[0], (unsigned int) buf[0]);

	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* When converting from an external buffer to a "don't care" buffer,
 * an external buffer is kept as is.
 */
static duk_ret_t test_6c(duk_context *ctx) {
	unsigned char buf[16];
	int i;
	unsigned char *p;
	duk_size_t sz;

	for (i = 0; i < 16; i++) {
		buf[i] = (unsigned char) i;
	}

	duk_push_external_buffer(ctx);
	duk_config_buffer(ctx, -1, (void *) buf, 16);

	p = (unsigned char *) duk_to_buffer(ctx, -1, &sz);
	printf("sz=%ld\n", (long) sz); fflush(stdout);
	p[0] = (unsigned char) 123;
	printf("p[0]=%u, buf[0]=%u\n", (unsigned int) p[0], (unsigned int) buf[0]);

	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1a);
	TEST_SAFE_CALL(test_1b);
	TEST_SAFE_CALL(test_2a);
	TEST_SAFE_CALL(test_2b);
	TEST_SAFE_CALL(test_3a);
	TEST_SAFE_CALL(test_3b);
	TEST_SAFE_CALL(test_4a);
	TEST_SAFE_CALL(test_4b);
	TEST_SAFE_CALL(test_5a);
	TEST_SAFE_CALL(test_5b);
	TEST_SAFE_CALL(test_6a);
	TEST_SAFE_CALL(test_6b);
	TEST_SAFE_CALL(test_6c);
}
