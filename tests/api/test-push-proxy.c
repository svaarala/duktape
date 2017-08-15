/*
 *  duk_push_proxy()
 */

/*===
*** test_basic (duk_safe_call)
top before: 6
top after: 5
duk_push_proxy() returned 4
proxy.foo=321
proxy.bar=321
final top: 5
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	duk_idx_t ret;

	(void) udata;

	duk_push_string(ctx, "dummy1");
	duk_push_string(ctx, "dummy2");
	duk_push_string(ctx, "dummy3");
	duk_push_string(ctx, "dummy4");

	duk_eval_string(ctx, "({ foo: 123 })");
	duk_eval_string(ctx, "({ get: function myget() { return 321; } })");
	printf("top before: %ld\n", (long) duk_get_top(ctx));
	ret = duk_push_proxy(ctx, 0);
	printf("top after: %ld\n", (long) duk_get_top(ctx));
	printf("duk_push_proxy() returned %ld\n", (long) ret);

	duk_get_prop_string(ctx, -1, "foo");
	printf("proxy.foo=%s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, "bar");
	printf("proxy.bar=%s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
