/*===
*** test_1 (duk_safe_call)
==> rc=1, result='TypeError: object required, found true (stack index -1)'
*** test_2 (duk_safe_call)
final top: 1
==> rc=0, result='undefined'
*** test_3 (duk_safe_call)
final top: 1
==> rc=0, result='undefined'
*** test_4 (duk_safe_call)
function
==> rc=1, result='TypeError: constructable required, found [object Function] (stack index -1)'
*** test_5 (duk_safe_call)
==> rc=1, result='TypeError: object required, found none (stack index 123)'
===*/

static duk_ret_t dummy_func(duk_context *ctx) {
	(void) ctx;
	return 0;
}

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_true(ctx);
	duk_require_constructable(ctx, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_2(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_c_function(ctx, dummy_func, 0);
	duk_require_constructable(ctx, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_3(duk_context *ctx, void *udata) {
	(void) udata;

	duk_eval_string(ctx, "(function MyConstructor() {})");
	duk_require_constructable(ctx, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_4(duk_context *ctx, void *udata) {
	(void) udata;

	duk_eval_string(ctx, "(function () { \n"
	                     "    // Literal getter is not constructable.\n"
	                     "    var O = { get foo() {} };\n"
	                     "    var pd = Object.getOwnPropertyDescriptor(O, 'foo');\n"
	                     "    print(typeof pd.get);\n"
	                     "    return pd.get;\n"
	                     "})()");
	duk_require_constructable(ctx, -1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_5(duk_context *ctx, void *udata) {
	(void) udata;

	duk_require_constructable(ctx, 123);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
	TEST_SAFE_CALL(test_5);
}
