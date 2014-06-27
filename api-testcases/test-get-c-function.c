/*===
duk_get_c_function == my_func: 1
index 3 -> NULL: 1
index DUK_INVALID_INDEX -> NULL: 1
===*/

static duk_ret_t my_func(duk_context *ctx) {
	return 0;
}

void test(duk_context *ctx) {
	duk_c_function funcptr;

	duk_push_c_function(ctx, my_func, 1 /*nargs*/);

	funcptr = duk_get_c_function(ctx, -1);
	printf("duk_get_c_function == my_func: %d\n", (funcptr == my_func ? 1 : 0));

	funcptr = duk_get_c_function(ctx, 3);
	printf("index 3 -> NULL: %d\n", (funcptr == NULL ? 1 : 0));

	funcptr = duk_get_c_function(ctx, DUK_INVALID_INDEX);
	printf("index DUK_INVALID_INDEX -> NULL: %d\n", (funcptr == NULL ? 1 : 0));
}
