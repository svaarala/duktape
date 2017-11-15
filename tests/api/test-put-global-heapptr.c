/*===
*** test_basic (duk_safe_call)
top: 1
top: 1
ret: 1
1.2.3
final top: 1
==> rc=0, result='undefined'
*** test_nonwritable (duk_safe_call)
top: 1
==> rc=1, result='TypeError: not writable'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	duk_bool_t ret;
	void *ptr;

	(void) udata;

	(void) duk_push_string(ctx, "myAppVersion");
	ptr = duk_get_heapptr(ctx, -1);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_push_string(ctx, "1.2.3");
	ret = duk_put_global_heapptr(ctx, ptr);
	printf("top: %ld\n", (long) duk_get_top(ctx));
	printf("ret: %ld\n", (long) ret);

	duk_eval_string_noresult(ctx, "print(myAppVersion);");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_nonwritable(duk_context *ctx, void *udata) {
	duk_bool_t ret;
	void *ptr;

	(void) udata;

	(void) duk_push_string(ctx, "nonWritable");
	ptr = duk_get_heapptr(ctx, -1);

	duk_eval_string_noresult(ctx,
		"Object.defineProperty(this, 'nonWritable', "
		"{ value: 'foo', writable: false, enumerable: false, configurable: false });"
	);

	printf("top: %ld\n", (long) duk_get_top(ctx));
	duk_push_string(ctx, "bar");
	ret = duk_put_global_heapptr(ctx, ptr);
	printf("top: %ld\n", (long) duk_get_top(ctx));
	printf("ret: %ld\n", (long) ret);

	duk_eval_string_noresult(ctx, "print(nonWritable);");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_nonwritable);
}
