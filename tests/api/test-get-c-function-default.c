/*===
duk_get_c_function == my_func: 1
index 3 -> dummy_func: 1
index DUK_INVALID_INDEX -> dummy_func: 1
===*/

static duk_ret_t my_func(duk_context *ctx) {
	(void) ctx;
	return 0;
}

static duk_ret_t dummy_func(duk_context *ctx) {
	(void) ctx;
	return 1;
}

void test(duk_context *ctx) {
	duk_c_function funcptr;

	duk_push_c_function(ctx, my_func, 1 /*nargs*/);

	funcptr = duk_get_c_function_default(ctx, -1, dummy_func);
	printf("duk_get_c_function == my_func: %d\n", (funcptr == my_func ? 1 : 0));

	funcptr = duk_get_c_function_default(ctx, 3, dummy_func);
	printf("index 3 -> dummy_func: %d\n", (funcptr == dummy_func ? 1 : 0));

	funcptr = duk_get_c_function_default(ctx, DUK_INVALID_INDEX, dummy_func);
	printf("index DUK_INVALID_INDEX -> dummy_func: %d\n", (funcptr == dummy_func ? 1 : 0));
}
