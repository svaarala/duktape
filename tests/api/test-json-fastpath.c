/*
 *  JSON fast path tests which cannot be covered from pure Ecmascript code.
 */

/*===
*** test_1 (duk_safe_call)
{uncovered:null,covered:|00000000|}
final top: 4
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx) {
	/* Bufferobject without cover. */
	duk_push_fixed_buffer(ctx, 4);
	duk_push_buffer_object(ctx, 0, 0, 100, DUK_BUFOBJ_UINT8ARRAY);
	duk_push_buffer_object(ctx, 0, 0, 4, DUK_BUFOBJ_UINT8ARRAY);

	duk_eval_string(ctx,
		"(function test(x, y) {\n"
		"    print(Duktape.enc('jx', { uncovered: x, covered: y }));\n"
		"})");
	duk_dup(ctx, 1);
	duk_dup(ctx, 2);
	duk_call(ctx, 2);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
