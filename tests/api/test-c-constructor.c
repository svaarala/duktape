/*===
*** test_1 (duk_safe_call)
inherited value
top at end: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t my_constructor(duk_context *ctx) {
	return 0;
}

static duk_ret_t test_1(duk_context *ctx) {
	duk_push_global_object(ctx);
	duk_push_c_function(ctx, my_constructor, 0);   /* constructor (function) */
	duk_push_object(ctx);                          /* prototype object -> [ global cons proto ] */
	duk_push_string(ctx, "inherited value");
	duk_put_prop_string(ctx, -2, "inherited");     /* set proto.inherited = "inherited value" */
	duk_put_prop_string(ctx, -2, "prototype");     /* set cons.prototype = proto; stack -> [ global cons ] */
	duk_put_prop_string(ctx, -2, "MyConstructor"); /* set global.MyConstructor = cons; stack -> [ global ] */
	duk_pop(ctx);

	duk_eval_string(ctx, "var obj = new MyConstructor(); print(obj.inherited);");
	duk_pop(ctx);

	printf("top at end: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
