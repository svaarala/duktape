/*
 *  Demonstrate replacing global object with a Proxy that returns dummy
 *  values for non-existent properties.
 */

/*===
*** test_1 (duk_safe_call)
GET Math
[object Math]
GET nonExistent
replaced
HAS print
GET print
HAS Math
GET Math
object
HAS print
GET print
HAS nonExistent
GET nonExistent
string
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	duk_eval_string(ctx,
		"new Proxy(this, {\n"
		"    get: function (targ, key, recv) { print('GET', key); if (key in targ) { return targ[key]; } else { return 'replaced'; } },\n"
		"    has: function (targ, key) { print('HAS', key); return true; }\n"
		"})");
	duk_set_global_object(ctx);

	duk_get_global_string(ctx, "Math");
	printf("%s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	duk_get_global_string(ctx, "nonExistent");
	printf("%s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	duk_eval_string_noresult(ctx, "print(typeof Math);");
	duk_eval_string_noresult(ctx, "print(typeof nonExistent);");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
