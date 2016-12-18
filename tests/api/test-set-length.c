/*===
*** test_basic (duk_safe_call)
["foo","bar","quux","bax"]
["foo"]
["foo",null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null]
{"foo":123}
{"foo":123,"length":123}
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	(void) udata;

	duk_eval_string(ctx, "[ 'foo', 'bar', 'quux', 'bax' ]");
	duk_dup(ctx, -1);
	printf("%s\n", duk_json_encode(ctx, -1));
	duk_pop(ctx);

	duk_set_length(ctx, -1, 1);
	duk_dup(ctx, -1);
	printf("%s\n", duk_json_encode(ctx, -1));
	duk_pop(ctx);

	duk_set_length(ctx, -1, 100);
	duk_dup(ctx, -1);
	printf("%s\n", duk_json_encode(ctx, -1));
	duk_pop(ctx);

	duk_pop(ctx);

	duk_eval_string(ctx, "({ foo: 123 })");
	duk_dup(ctx, -1);
	printf("%s\n", duk_json_encode(ctx, -1));
	duk_pop(ctx);

	duk_set_length(ctx, -1, 123);
	duk_dup(ctx, -1);
	printf("%s\n", duk_json_encode(ctx, -1));
	duk_pop(ctx);

	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
