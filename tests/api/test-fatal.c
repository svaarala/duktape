/*===
my_func
fatal error: my reason
===*/

void my_fatal_handler(void *udata, const char *msg) {
	(void) udata;

	printf("fatal error: %s\n", msg);

	/* A fatal error handler must not return, so exit here */
	exit(0);
}

static duk_ret_t my_func(duk_context *ctx, void *udata) {
	(void) udata;

	printf("my_func\n");
	duk_fatal(ctx, "my reason");
	printf("never here\n");
	return 0;
}

void test(duk_context *ctx) {
	duk_context *new_ctx;

	new_ctx = duk_create_heap(NULL, NULL, NULL, NULL, my_fatal_handler);
	duk_safe_call(new_ctx, my_func, NULL, 0, 1);
	printf("duk_safe_call() returned, should not happen\n");
}
