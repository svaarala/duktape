/*===
FIXME
===*/

/* FIXME: at the moment a deterministic testcase is not possible because
 * the debug line contains the context pointer (%p formatted).
 */

static int test_1(duk_context *ctx) {
	duk_push_int(ctx, 123);
	duk_eval_string(ctx, "'foo\\u1234bar'");
	duk_eval_string(ctx, "({ foo: 123, bar: [ 1, 2, 3 ]})");
	duk_eval_string(ctx, "([ 1, 2, 3 ])");
	duk_dump_context_stdout(ctx);
	printf("final top: %d\n", duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
