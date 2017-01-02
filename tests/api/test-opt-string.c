static void dump(const unsigned char *buf) {
	const unsigned char *p = buf;
	int first = 1;

	if (!buf) {
		printf("null\n");
		return;
	}

	printf("[");
	while (*p) {
		if (first) {
			first = 0;
		} else {
			printf(" ");
		}
		printf("%02x", (int) (*p));
		p++;
	}
	printf("]\n");
}

static duk_ret_t safe_helper_string(duk_context *ctx, void *udata) {
	duk_idx_t idx = (duk_idx_t) udata & 0xffffffffUL;

	dump((const unsigned char *) duk_opt_string(ctx, idx, "default"));
	return 0;
}

static duk_ret_t safe_helper_lstring1(duk_context *ctx, void *udata) {
	duk_idx_t idx = (duk_idx_t) udata & 0xffffffffUL;
	const char *buf;
	size_t len;

	len = (size_t) 0xdeadbeefUL;
	buf = duk_opt_lstring(ctx, idx, &len, "default!", 7);
	printf("length %lu: ", (unsigned long) len);
	dump((const unsigned char *) buf);
	return 0;
}

static duk_ret_t safe_helper_lstring2(duk_context *ctx, void *udata) {
	duk_idx_t idx = (duk_idx_t) udata & 0xffffffffUL;
	const char *buf;

	buf = duk_opt_lstring(ctx, idx, NULL, "default!", 7);
	dump((const unsigned char *) buf);
	return 0;
}

/*===
*** test_opt_string (duk_safe_call)
top: 10
index 0: [64 65 66 61 75 6c 74]
index 1: TypeError: string required, found null (stack index 1)
index 2: TypeError: string required, found true (stack index 2)
index 3: TypeError: string required, found false (stack index 3)
index 4: []
index 5: [66 6f 6f]
index 6: [66 6f 6f]
index 7: [e1 88 b4 78 79 7a]
index 8: TypeError: string required, found NaN (stack index 8)
index 9: TypeError: string required, found [object Object] (stack index 9)
index 10: [64 65 66 61 75 6c 74]
==> rc=0, result='undefined'
===*/

static duk_ret_t test_opt_string(duk_context *ctx, void *udata) {
	duk_idx_t i, n;
	duk_int_t rc;

	(void) udata;

	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_true(ctx);
	duk_push_false(ctx);
	duk_push_string(ctx, "");
	duk_push_string(ctx, "foo");
	duk_push_lstring(ctx, "foo\0bar", 7);
	duk_push_string(ctx, "\xe1\x88\xb4xyz");  /* 4 chars, first char utf-8 encoded U+1234 */
	duk_push_nan(ctx);
	duk_push_object(ctx);

	n = duk_get_top(ctx);
	printf("top: %ld\n", (long) n);
	for (i = 0; i <= n; i++) {
		printf("index %ld: ", (long) i);
		rc = duk_safe_call(ctx, safe_helper_string, (void *) i, 0, 1);
		if (rc != DUK_EXEC_SUCCESS) {
			printf("%s\n", duk_safe_to_string(ctx, -1));
		}
		duk_pop(ctx);
	}

	return 0;
}

/*===
*** test_opt_lstring (duk_safe_call)
top: 10
index 0: length 7: [64 65 66 61 75 6c 74 21]
index 0: [64 65 66 61 75 6c 74 21]
index 1: TypeError: string required, found null (stack index 1)
index 1: TypeError: string required, found null (stack index 1)
index 2: TypeError: string required, found true (stack index 2)
index 2: TypeError: string required, found true (stack index 2)
index 3: TypeError: string required, found false (stack index 3)
index 3: TypeError: string required, found false (stack index 3)
index 4: length 0: []
index 4: []
index 5: length 3: [66 6f 6f]
index 5: [66 6f 6f]
index 6: length 7: [66 6f 6f]
index 6: [66 6f 6f]
index 7: length 6: [e1 88 b4 78 79 7a]
index 7: [e1 88 b4 78 79 7a]
index 8: TypeError: string required, found NaN (stack index 8)
index 8: TypeError: string required, found NaN (stack index 8)
index 9: TypeError: string required, found [object Object] (stack index 9)
index 9: TypeError: string required, found [object Object] (stack index 9)
index 10: length 7: [64 65 66 61 75 6c 74 21]
index 10: [64 65 66 61 75 6c 74 21]
==> rc=0, result='undefined'
===*/

static duk_ret_t test_opt_lstring(duk_context *ctx, void *udata) {
	duk_idx_t i, n;
	duk_int_t rc;

	(void) udata;

	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_true(ctx);
	duk_push_false(ctx);
	duk_push_string(ctx, "");
	duk_push_string(ctx, "foo");
	duk_push_lstring(ctx, "foo\0bar", 7);
	duk_push_string(ctx, "\xe1\x88\xb4xyz");  /* 4 chars, first char utf-8 encoded U+1234 */
	duk_push_nan(ctx);
	duk_push_object(ctx);

	n = duk_get_top(ctx);
	printf("top: %ld\n", (long) n);
	for (i = 0; i <= n; i++) {
		printf("index %ld: ", (long) i);
		rc = duk_safe_call(ctx, safe_helper_lstring1, (void *) i, 0, 1);
		if (rc != DUK_EXEC_SUCCESS) {
			printf("%s\n", duk_safe_to_string(ctx, -1));
		}
		duk_pop(ctx);

		printf("index %ld: ", (long) i);
		rc = duk_safe_call(ctx, safe_helper_lstring2, (void *) i, 0, 1);
		if (rc != DUK_EXEC_SUCCESS) {
			printf("%s\n", duk_safe_to_string(ctx, -1));
		}
		duk_pop(ctx);
	}

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_opt_string);
	TEST_SAFE_CALL(test_opt_lstring);
}
