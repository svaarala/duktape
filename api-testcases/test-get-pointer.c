/*===
top: 9
index 0, pointer (nil)
index 1, pointer (nil)
index 2, pointer (nil)
index 3, pointer (nil)
index 4, pointer (nil)
index 5, pointer (nil)
index 6, pointer (nil)
index 7, pointer (nil)
index 8, pointer 0xdeadbeef
===*/

void test(duk_context *ctx) {
	duk_idx_t i, n;

	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_true(ctx);
	duk_push_false(ctx);
	duk_push_string(ctx, "foo");
	duk_push_int(ctx, 123);
	duk_push_object(ctx);
	duk_push_pointer(ctx, (void *) NULL);
	duk_push_pointer(ctx, (void *) 0xdeadbeef);

	n = duk_get_top(ctx);
	printf("top: %ld\n", (long) n);
	for (i = 0; i < n; i++) {
		void *ptr = duk_get_pointer(ctx, i);
		printf("index %ld, pointer %p\n", (long) i, ptr);
	}
}
