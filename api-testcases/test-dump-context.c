/*===
*** test_1 (duk_safe_call)
ctx: top=4, stack=[123,"foo\u1234bar",{foo:123,bar:[1,2,3]},[1,2,3]]
final top: 4
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	duk_push_int(ctx, 123);
	duk_eval_string(ctx, "'foo\\u1234bar'");
	duk_eval_string(ctx, "({ foo: 123, bar: [ 1, 2, 3 ]})");
	duk_eval_string(ctx, "([ 1, 2, 3 ])");
	duk_dump_context_stdout(ctx);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
