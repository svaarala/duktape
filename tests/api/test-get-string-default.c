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

/*===
*** test_string (duk_safe_call)
top: 10
index 0: [64 65 66 61 75 6c 74]
index 1: [64 65 66 61 75 6c 74]
index 2: [64 65 66 61 75 6c 74]
index 3: [64 65 66 61 75 6c 74]
index 4: []
index 5: [66 6f 6f]
index 6: [66 6f 6f]
index 7: [e1 88 b4 78 79 7a]
index 8: [64 65 66 61 75 6c 74]
index 9: [64 65 66 61 75 6c 74]
index 10: [64 65 66 61 75 6c 74]
==> rc=0, result='undefined'
===*/

static duk_ret_t test_string(duk_context *ctx, void *udata) {
	duk_idx_t i, n;

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
		dump((const unsigned char *) duk_get_string_default(ctx, i, "default"));
	}

	return 0;
}

/*===
*** test_lstring (duk_safe_call)
top: 10
index 0: length 7: [64 65 66 61 75 6c 74 21]
index 0: [64 65 66 61 75 6c 74 21]
index 1: length 7: [64 65 66 61 75 6c 74 21]
index 1: [64 65 66 61 75 6c 74 21]
index 2: length 7: [64 65 66 61 75 6c 74 21]
index 2: [64 65 66 61 75 6c 74 21]
index 3: length 7: [64 65 66 61 75 6c 74 21]
index 3: [64 65 66 61 75 6c 74 21]
index 4: length 0: []
index 4: []
index 5: length 3: [66 6f 6f]
index 5: [66 6f 6f]
index 6: length 7: [66 6f 6f]
index 6: [66 6f 6f]
index 7: length 6: [e1 88 b4 78 79 7a]
index 7: [e1 88 b4 78 79 7a]
index 8: length 7: [64 65 66 61 75 6c 74 21]
index 8: [64 65 66 61 75 6c 74 21]
index 9: length 7: [64 65 66 61 75 6c 74 21]
index 9: [64 65 66 61 75 6c 74 21]
==> rc=0, result='undefined'
===*/

static duk_ret_t test_lstring(duk_context *ctx, void *udata) {
	duk_idx_t i, n;

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
	for (i = 0; i < n; i++) {
		const char *buf;
		size_t len;

		len = (size_t) 0xdeadbeefUL;
		buf = duk_get_lstring_default(ctx, i, &len, "default!", 7);
		printf("index %ld: length %lu: ",
		       (long) i, (unsigned long) len);
		dump((const unsigned char *) buf);

		buf = duk_get_lstring_default(ctx, i, NULL, "default!", 7);
		printf("index %ld: ", (long) i);
		dump((const unsigned char *) buf);
	}

	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_string);
	TEST_SAFE_CALL(test_lstring);
}
