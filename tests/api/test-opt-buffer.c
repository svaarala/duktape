static duk_ret_t safe_helper1(duk_context *ctx, void *udata) {
	duk_idx_t idx = (duk_idx_t) udata & 0xffffffffUL;
	void *buf;
	duk_size_t len;

	len = (duk_size_t) 0xdeadbeefUL;
	buf = duk_opt_buffer(ctx, idx, &len, (void *) 0x1357acefUL, (duk_size_t) 0x87654321UL);

	printf("index %ld: length %lu, ptr-is-NULL %d, ptr-is-0x1357acef %d\n",
	       (long) idx, (unsigned long) len, (buf == NULL ? 1 : 0), (buf == 0x1357acefUL));
	return 0;
}

static duk_ret_t safe_helper2(duk_context *ctx, void *udata) {
	duk_idx_t idx = (duk_idx_t) udata & 0xffffffffUL;
	void *buf;

	buf = duk_opt_buffer(ctx, idx, NULL, (void *) 0x1357acefUL, (duk_size_t) 0x87654321UL);

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
index 12: TypeError: buffer required, found [object ArrayBuffer] (stack index 12)
index 12: TypeError: buffer required, found [object ArrayBuffer] (stack index 12)
index 13: TypeError: buffer required, found [object Uint32Array] (stack index 13)
index 13: TypeError: buffer required, found [object Uint32Array] (stack index 13)
index 14: TypeError: buffer required, found [object DataView] (stack index 14)
index 14: TypeError: buffer required, found [object DataView] (stack index 14)
index 15: TypeError: buffer required, found [object Uint32Array] (stack index 15)
index 15: TypeError: buffer required, found [object Uint32Array] (stack index 15)
index 16: TypeError: buffer required, found [object Uint8Array] (stack index 16)
index 16: TypeError: buffer required, found [object Uint8Array] (stack index 16)
index 17: TypeError: buffer required, found [object Uint8Array] (stack index 17)
index 17: TypeError: buffer required, found [object Uint8Array] (stack index 17)
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

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
