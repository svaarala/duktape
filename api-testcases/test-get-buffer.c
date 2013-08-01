/*===
top: 12
index 0: length 0
index 1: length 0
index 2: length 0
index 3: length 0
index 4: length 0
index 5: length 0
index 6: length 0
index 7: length 0
index 8: length 0
index 9: length 1024, ptr-is-NULL 0
index 10: length 0
index 11: length 2048, ptr-is-NULL 0
===*/

void test(duk_context *ctx) {
	int i, n;

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

	n = duk_get_top(ctx);
	printf("top: %d\n", n);
	for (i = 0; i < n; i++) {
		void *buf;
		size_t len;

		len = (size_t) 0xdeadbeef;
		buf = duk_get_buffer(ctx, i, &len);
		if (len == 0) {
			/* avoid printing 'buf' if len is zero, as it is not predictable */
			printf("index %d: length 0\n", i);
		} else {
			printf("index %d: length %u, ptr-is-NULL %d\n", i, (unsigned int) len, (buf == NULL ? 1 : 0));
		}
	}
}

