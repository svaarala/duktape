/*===
*** test_basic (duk_safe_call)
0: 1
1: 1
2: 1
3: 1
4: 1
5: 1
6: 1
7: 1
8: 1
9: 1
10: 1
11: 1
12: 1
13: 1
14: 1
15: 0
16: 0
17: 0
final top: 18
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	duk_idx_t i, n;

	(void) udata;

	duk_push_fixed_buffer(ctx, 128);
	duk_push_dynamic_buffer(ctx, 256);
	duk_push_external_buffer(ctx);

	duk_eval_string(ctx, "new Buffer(10)");
	duk_eval_string(ctx, "new ArrayBuffer(10)");
	duk_eval_string(ctx, "new Uint8Array(10)");
	duk_eval_string(ctx, "new Uint8ClampedArray(10)");
	duk_eval_string(ctx, "new Int8Array(10)");
	duk_eval_string(ctx, "new Uint16Array(10)");
	duk_eval_string(ctx, "new Int16Array(10)");
	duk_eval_string(ctx, "new Uint32Array(10)");
	duk_eval_string(ctx, "new Int32Array(10)");
	duk_eval_string(ctx, "new Float32Array(10)");
	duk_eval_string(ctx, "new Float64Array(10)");

	duk_eval_string(ctx, "new DataView(new ArrayBuffer(10))");

	/* False for non-buffer values. */

	duk_push_null(ctx);
	duk_push_string(ctx, "foo");

	/* Also false for objects that inherit from buffer objects. */

	duk_eval_string(ctx, "Object.create(new Uint8Array(10))");

	for (i = 0, n = duk_get_top(ctx); i < n; i++) {
		printf("%ld: %ld\n", (long) i, (long) duk_is_buffer_data(ctx, i));
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
