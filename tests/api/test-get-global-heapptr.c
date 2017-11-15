/*===
*** test_basic (duk_safe_call)
top: 1
top: 2
ret: 1
duk_is_function: 1
final top: 2
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	duk_bool_t ret;
	void *ptr;

	(void) udata;

	(void) duk_push_string(ctx, "encodeURIComponent");
	ptr = duk_get_heapptr(ctx, -1);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	ret = duk_get_global_heapptr(ctx, ptr);
	printf("top: %ld\n", (long) duk_get_top(ctx));
	printf("ret: %ld\n", (long) ret);
	printf("duk_is_function: %ld\n", duk_is_function(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
