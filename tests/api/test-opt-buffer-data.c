static duk_ret_t safe_helper1(duk_context *ctx, void *udata) {
	duk_idx_t idx = (duk_idx_t) udata & 0xffffffffUL;
	void *buf;
	duk_size_t len;

	len = (duk_size_t) 0xdeadbeefUL;
	buf = duk_opt_buffer_data(ctx, idx, &len, (void *) 0x1357acefUL, (duk_size_t) 0x87654321UL);

	printf("index %ld: length %lu, ptr-is-NULL %d, ptr-is-0x1357acef %d\n",
	       (long) idx, (unsigned long) len, (buf == NULL ? 1 : 0), (buf == 0x1357acefUL));
	return 0;
}

static duk_ret_t safe_helper2(duk_context *ctx, void *udata) {
	duk_idx_t idx = (duk_idx_t) udata & 0xffffffffUL;
	void *buf;

	buf = duk_opt_buffer_data(ctx, idx, NULL, (void *) 0x1357acefUL, (duk_size_t) 0x87654321UL);

	printf("index %ld: ptr-is-NULL %d, ptr-is-0x1357acef %d\n",
	       (long) idx, (buf == NULL ? 1 : 0), (buf == 0x1357acefUL));
	return 0;
}

/*===
*** test_basic (duk_safe_call)
top: 18
index 0: length 2271560481, ptr-is-NULL 0, ptr-is-0x1357acef 1
index 0: ptr-is-NULL 0, ptr-is-0x1357acef 1
index 1: TypeError: buffer required, found null (stack index 1)
index 1: TypeError: buffer required, found null (stack index 1)
index 2: TypeError: buffer required, found true (stack index 2)
index 2: TypeError: buffer required, found true (stack index 2)
index 3: TypeError: buffer required, found false (stack index 3)
index 3: TypeError: buffer required, found false (stack index 3)
index 4: TypeError: buffer required, found '' (stack index 4)
index 4: TypeError: buffer required, found '' (stack index 4)
index 5: TypeError: buffer required, found 'foo' (stack index 5)
index 5: TypeError: buffer required, found 'foo' (stack index 5)
index 6: TypeError: buffer required, found 123 (stack index 6)
index 6: TypeError: buffer required, found 123 (stack index 6)
index 7: TypeError: buffer required, found [object Object] (stack index 7)
index 7: TypeError: buffer required, found [object Object] (stack index 7)
index 8: length 0, ptr-is-NULL 0, ptr-is-0x1357acef 0
index 8: ptr-is-NULL 0, ptr-is-0x1357acef 0
index 9: length 1024, ptr-is-NULL 0, ptr-is-0x1357acef 0
index 9: ptr-is-NULL 0, ptr-is-0x1357acef 0
index 10: length 0, ptr-is-NULL 1, ptr-is-0x1357acef 0
index 10: ptr-is-NULL 1, ptr-is-0x1357acef 0
index 11: length 2048, ptr-is-NULL 0, ptr-is-0x1357acef 0
index 11: ptr-is-NULL 0, ptr-is-0x1357acef 0
index 12: length 16, ptr-is-NULL 0, ptr-is-0x1357acef 0
index 12: ptr-is-NULL 0, ptr-is-0x1357acef 0
index 13: length 64, ptr-is-NULL 0, ptr-is-0x1357acef 0
index 13: ptr-is-NULL 0, ptr-is-0x1357acef 0
index 14: length 16, ptr-is-NULL 0, ptr-is-0x1357acef 0
index 14: ptr-is-NULL 0, ptr-is-0x1357acef 0
index 15: length 12, ptr-is-NULL 0, ptr-is-0x1357acef 0
index 15: ptr-is-NULL 0, ptr-is-0x1357acef 0
index 16: length 8, ptr-is-NULL 0, ptr-is-0x1357acef 0
index 16: ptr-is-NULL 0, ptr-is-0x1357acef 0
index 17: length 3, ptr-is-NULL 0, ptr-is-0x1357acef 0
index 17: ptr-is-NULL 0, ptr-is-0x1357acef 0
index 18: length 2271560481, ptr-is-NULL 0, ptr-is-0x1357acef 1
index 18: ptr-is-NULL 0, ptr-is-0x1357acef 1
final top: 18
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	duk_idx_t i, n;
	duk_int_t rc;

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
		rc = duk_safe_call(ctx, safe_helper1, (void *) i, 0, 1);
		if (rc != DUK_EXEC_SUCCESS) {
			printf("index %ld: %s\n", (long) i, duk_safe_to_string(ctx, -1));
		}
		duk_pop(ctx);

		rc = duk_safe_call(ctx, safe_helper2, (void *) i, 0, 1);
		if (rc != DUK_EXEC_SUCCESS) {
			printf("index %ld: %s\n", (long) i, duk_safe_to_string(ctx, -1));
		}
		duk_pop(ctx);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_uncovered (duk_safe_call)
==> rc=1, result='TypeError: buffer required, found [object ArrayBuffer] (stack index -1)'
===*/

static duk_ret_t test_uncovered(duk_context *ctx, void *udata) {
	void *ptr;
	duk_size_t sz;

	(void) udata;

	ptr = duk_push_dynamic_buffer(ctx, 1024);
	memset(ptr, 0, 1024);
	duk_push_buffer_object(ctx, -1, 0, 1024, DUK_BUFOBJ_ARRAYBUFFER);
	duk_resize_buffer(ctx, -2, 1);  /* 1024 -> 1 byte(s) */

	ptr = (void *) 0xdeadbeefUL;
	sz = (void *) 0x12345678UL;
	ptr = duk_opt_buffer_data(ctx, -1, &sz, 0x87654321UL, 0xabcdef99UL);
	printf("ptr: NULL=%d, 0xdeadbeef=%d 0x87654321=%d\n", ptr == NULL, ptr == (void *) 0xdeadbeefUL, ptr == (void *) 0x87654321UL);
	printf("sz: 0=%d, 0x12345678=%d 0xabcdef99=%d\n", sz == 0, sz == 0x12345678UL, sz == 0xabcdef99);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_uncovered);
}
