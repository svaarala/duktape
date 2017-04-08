/*===
*** test_basic (duk_safe_call)
top: 12
index 0: dummy 1
index 1: TypeError: thread required, found null (stack index 1)
index 2: TypeError: thread required, found true (stack index 2)
index 3: TypeError: thread required, found false (stack index 3)
index 4: TypeError: thread required, found '' (stack index 4)
index 5: TypeError: thread required, found 'foo' (stack index 5)
index 6: TypeError: thread required, found 'true' (stack index 6)
index 7: TypeError: thread required, found 'false' (stack index 7)
index 8: TypeError: thread required, found 0 (stack index 8)
index 9: TypeError: thread required, found 123 (stack index 9)
index 10: TypeError: thread required, found [object Object] (stack index 10)
index 11: dummy 0
index 12: dummy 1
==> rc=0, result='undefined'
===*/

static duk_ret_t safe_helper(duk_context *ctx, void *udata) {
	duk_idx_t idx = (duk_idx_t) udata & 0xffffffffUL;
	duk_context *v;
	char dummy;

	v = duk_opt_context(ctx, idx, (duk_context *) &dummy);
	printf("index %ld: dummy %d\n",
	       (long) idx, (v == (duk_context *) &dummy));
	return 0;
}

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
	duk_push_string(ctx, "true");
	duk_push_string(ctx, "false");
	duk_push_int(ctx, 0);
	duk_push_int(ctx, 123);
	duk_push_object(ctx);
	(void) duk_push_thread(ctx);

	n = duk_get_top(ctx);
	printf("top: %ld\n", (long) n);
	for (i = 0; i <= n; i++) {  /* End out-of-bounds on purpose. */
		rc = duk_safe_call(ctx, safe_helper, (void *) i, 0, 1);
		if (rc != DUK_EXEC_SUCCESS) {
			printf("index %ld: %s\n", (long) i, duk_safe_to_string(ctx, -1));
		}
		duk_pop(ctx);
	}

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
