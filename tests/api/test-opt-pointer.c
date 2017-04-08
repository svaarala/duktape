static duk_ret_t safe_helper(duk_context *ctx, void *udata) {
	duk_idx_t idx = (duk_idx_t) udata & 0xffffffffUL;
	void *ptr;

	ptr = duk_opt_pointer(ctx, idx, (void *) 0x87654321UL);
	printf("index %ld, pointer %p\n", (long) idx, ptr);
	return 0;
}

/*===
*** test_basic (duk_safe_call)
top: 9
index 0, pointer 0x87654321
index 1: TypeError: pointer required, found null (stack index 1)
index 2: TypeError: pointer required, found true (stack index 2)
index 3: TypeError: pointer required, found false (stack index 3)
index 4: TypeError: pointer required, found 'foo' (stack index 4)
index 5: TypeError: pointer required, found 123 (stack index 5)
index 6: TypeError: pointer required, found [object Object] (stack index 6)
index 7, pointer (nil)
index 8, pointer 0xdeadbeef
index 9, pointer 0x87654321
final top: 9
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
	duk_push_string(ctx, "foo");
	duk_push_int(ctx, 123);
	duk_push_object(ctx);
	duk_push_pointer(ctx, (void *) NULL);
	duk_push_pointer(ctx, (void *) 0xdeadbeefUL);

	n = duk_get_top(ctx);
	printf("top: %ld\n", (long) n);
	for (i = 0; i <= n; i++) {
		rc = duk_safe_call(ctx, safe_helper, (void *) i, 0, 1);
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
