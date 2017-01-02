/*===
*** test_basic (duk_safe_call)
index 0: is dummy: 1
index 1: is dummy: 1
index 2: is dummy: 0
index 3: is dummy: 0
index 4: is dummy: 0
index 5: is dummy: 1
final top: 5
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	duk_idx_t i, n;

	(void) udata;

	duk_push_undefined(ctx);
	duk_push_null(ctx);
	duk_push_string(ctx, "dummy");
	duk_eval_string(ctx, "({ foo: 123 })");
	duk_eval_string(ctx, "([ 1, 2, 3 ])");

	n = duk_get_top(ctx);
	for (i = 0; i <= n; i++) {
		printf("index %ld: is dummy: %d\n", (long) i, duk_get_heapptr_default(ctx, i, (void *) 123456789) == (void *) 123456789 ? 1 : 0);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
