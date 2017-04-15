/*===
*** test_basic (duk_safe_call)
top: 18
index 0: length 2271560481, ptr-is-12345678 1, ptr-is-NULL 0
index 1: length 2271560481, ptr-is-12345678 1, ptr-is-NULL 0
index 2: length 2271560481, ptr-is-12345678 1, ptr-is-NULL 0
index 3: length 2271560481, ptr-is-12345678 1, ptr-is-NULL 0
index 4: length 2271560481, ptr-is-12345678 1, ptr-is-NULL 0
index 5: length 2271560481, ptr-is-12345678 1, ptr-is-NULL 0
index 6: length 2271560481, ptr-is-12345678 1, ptr-is-NULL 0
index 7: length 2271560481, ptr-is-12345678 1, ptr-is-NULL 0
index 8: length 0, ptr-is-12345678 0
index 9: length 1024, ptr-is-12345678 0, ptr-is-NULL 0
index 10: length 0, ptr-is-12345678 0
index 11: length 2048, ptr-is-12345678 0, ptr-is-NULL 0
index 12: length 2271560481, ptr-is-12345678 1, ptr-is-NULL 0
index 13: length 2271560481, ptr-is-12345678 1, ptr-is-NULL 0
index 14: length 2271560481, ptr-is-12345678 1, ptr-is-NULL 0
index 15: length 2271560481, ptr-is-12345678 1, ptr-is-NULL 0
index 16: length 2271560481, ptr-is-12345678 1, ptr-is-NULL 0
index 17: length 2271560481, ptr-is-12345678 1, ptr-is-NULL 0
index 18: length 2271560481, ptr-is-12345678 1, ptr-is-NULL 0
manual test: ptr=123456789, len=987654321
final top: 18
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	duk_idx_t i, n;
	void *buf;
	duk_size_t len;

	(void) udata;

	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_true(ctx);
	duk_push_false(ctx);
	duk_push_string(ctx, "");
	duk_push_string(ctx, "foo");
	duk_push_int(ctx, 123);
	duk_push_object(ctx);
	duk_push_fixed_buffer(ctx, 0);
	duk_push_fixed_buffer(ctx, 1024);
	duk_push_dynamic_buffer(ctx, 0);
	duk_push_dynamic_buffer(ctx, 2048);
	duk_eval_string(ctx, "(function () { return new ArrayBuffer(16); })()");
	duk_eval_string(ctx, "(function () { return new Uint32Array(16); })()");
	duk_eval_string(ctx, "(function () { return new DataView(new ArrayBuffer(16)); })()");
	duk_eval_string(ctx, "(function () { return new Uint32Array(16).subarray(3, 6); })()");
	duk_eval_string(ctx, "(function () { return new Buffer('ABCDEFGH'); })()");
	duk_eval_string(ctx, "(function () { return new Buffer('ABCDEFGH').slice(3, 6); })()");

	n = duk_get_top(ctx);
	printf("top: %ld\n", (long) n);
	for (i = 0; i <= n; i++) {
		len = (duk_size_t) 0xdeadbeefUL;
		buf = duk_get_buffer_default(ctx, i, &len, (void *) 0x12345678UL, (duk_size_t) 0x87654321UL);
		if (len == 0) {
			/* avoid printing 'buf' if len is zero, as it is not predictable */
			printf("index %ld: length %lu, ptr-is-12345678 %d\n",
			       (long) i, (unsigned long) len, (buf == (void *) 0x12345678UL));
		} else {
			printf("index %ld: length %lu, ptr-is-12345678 %d, ptr-is-NULL %d\n",
			       (long) i, (unsigned long) len, (buf == (void *) 0x12345678UL), (buf == NULL ? 1 : 0));
		}
	}

	buf = duk_get_buffer_default(ctx, 0, &len, (void *) 123456789UL, (duk_size_t) 987654321UL);
	printf("manual test: ptr=%lu, len=%lu\n", (unsigned long) buf, (unsigned long) len);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_null_ptr (duk_safe_call)
p is not NULL
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_null_ptr(duk_context *ctx, void *udata) {
	void *p;

	(void) udata;

	duk_push_fixed_buffer(ctx, 1024);

	p = duk_get_buffer_default(ctx, -1, NULL, (void *) 0x12345678UL, (duk_size_t) 0x87654321UL);
	if (p) {
		printf("p is not NULL\n");
	} else {
		printf("p is NULL\n");
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_invalid_index (duk_safe_call)
p is not NULL, sz=2271560481
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_invalid_index(duk_context *ctx, void *udata) {
	void *p;
	duk_size_t sz;

	(void) udata;

	sz = (duk_size_t) 0xdeadbeefUL;
	p = duk_get_buffer_default(ctx, -1, &sz, (void *) 0x12345678UL, (duk_size_t) 0x87654321UL);
	if (p) {
		printf("p is not NULL, sz=%lu\n", (unsigned long) sz);
	} else {
		printf("p is NULL\n");
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_buffer_object (duk_safe_call)
p is not NULL, sz=2271560481
==> rc=0, result='undefined'
===*/

static duk_ret_t test_buffer_object(duk_context *ctx, void *udata) {
	void *p;
	duk_size_t sz;

	(void) udata;

	/* duk_get_buffer_default() doesn't accept a buffer object */

	duk_set_top(ctx, 0);
	duk_eval_string(ctx, "new ArrayBuffer(16)");
	sz = (duk_size_t) 0xdeadbeefUL;
	p = duk_get_buffer_default(ctx, -1, &sz, (void *) 0x12345678UL, (duk_size_t) 0x87654321UL);
	if (p) {
		printf("p is not NULL, sz=%lu\n", (unsigned long) sz);
	} else {
		printf("p is NULL\n");
	}

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_null_ptr);
	TEST_SAFE_CALL(test_invalid_index);
	TEST_SAFE_CALL(test_buffer_object);
}
