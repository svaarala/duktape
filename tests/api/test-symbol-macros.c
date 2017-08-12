/*===
*** test_1 (duk_safe_call)
Ecma myGlobal: 1002
C myHidden: 1001
C myGlobal: 1002
C myLocal: 1003
C myWellKnown: 1004
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_object(ctx);
	duk_push_uint(ctx, 1001);
	duk_put_prop_string(ctx, -2, "\xFF" "myHidden");
	duk_push_uint(ctx, 1002);
	duk_put_prop_string(ctx, -2, "\x80" "myGlobal");
	duk_push_uint(ctx, 1003);
	duk_put_prop_string(ctx, -2, "\x81" "myLocal" "\xFF" "!123");
	duk_push_uint(ctx, 1004);
	duk_put_prop_string(ctx, -2, "\x81" "myWellKnown" "\xFF");

	duk_eval_string(ctx,
		"(function (o) {\n"
		"    print('Ecma myGlobal:', o[Symbol.for('myGlobal')]);\n"
		"})\n");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_get_prop_string(ctx, -1, DUK_HIDDEN_SYMBOL("myHidden"));
	printf("C myHidden: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
	duk_get_prop_string(ctx, -1, DUK_GLOBAL_SYMBOL("myGlobal"));
	printf("C myGlobal: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
	duk_get_prop_string(ctx, -1, DUK_LOCAL_SYMBOL("myLocal", "!123"));
	printf("C myLocal: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);
	duk_get_prop_string(ctx, -1, DUK_WELLKNOWN_SYMBOL("myWellKnown"));
	printf("C myWellKnown: %s\n", duk_safe_to_string(ctx, -1));
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
}
