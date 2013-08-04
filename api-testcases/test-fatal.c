/*===
my_func
fatal error: 123456
===*/

void my_fatal_handler(duk_context *ctx, int code) {
	printf("fatal error: %d\n", code);

	/* A fatal error handler must not return, so exit here */
	exit(0);
}

int my_func(duk_context *ctx) {
	printf("my_func\n");
	duk_fatal(ctx, 123456);
	printf("never here\n");
	return 0;
}

void test(duk_context *ctx) {
	duk_context *new_ctx;

	new_ctx = duk_create_heap(NULL, NULL, NULL, NULL, my_fatal_handler);
	duk_safe_call(new_ctx, my_func, 0, 1, DUK_INVALID_INDEX);
	printf("duk_safe_call() returned, should not happen\n");
}


