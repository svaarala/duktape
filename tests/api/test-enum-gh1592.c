/*===
*** test_1 (duk_safe_call)
key: foo -> bar
final top: 2
==> rc=0, result='undefined'
*** test_2 (duk_safe_call)
final top: 2
==> rc=0, result='undefined'
*** test_3 (duk_safe_call)
key: foo -> bar
final top: 2
==> rc=0, result='undefined'
*** test_4 (duk_safe_call)
final top: 2
==> rc=0, result='undefined'
===*/

static void setup(duk_context *ctx) {
	duk_eval_string(ctx, "new Proxy({ foo: 'bar' }, {});");
}

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	setup(ctx);

	duk_enum(ctx, 0, 0 /*flags*/);
	while (duk_next(ctx, 1, 1)) {
		printf("key: %s -> %s\n", duk_safe_to_string(ctx, -2), duk_safe_to_string(ctx, -1));
		duk_pop_2(ctx);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_2(duk_context *ctx, void *udata) {
	(void) udata;

	setup(ctx);

	duk_enum(ctx, 0, DUK_ENUM_NO_PROXY_BEHAVIOR /*flags*/);
	while (duk_next(ctx, 1, 1)) {
		printf("key: %s -> %s\n", duk_safe_to_string(ctx, -2), duk_safe_to_string(ctx, -1));
		duk_pop_2(ctx);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_3(duk_context *ctx, void *udata) {
	(void) udata;

	setup(ctx);

	duk_enum(ctx, 0, DUK_ENUM_INCLUDE_HIDDEN /*flags*/);
	while (duk_next(ctx, 1, 1)) {
		printf("key: %s -> %s\n", duk_safe_to_string(ctx, -2), duk_safe_to_string(ctx, -1));
		duk_pop_2(ctx);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_4(duk_context *ctx, void *udata) {
	(void) udata;

	setup(ctx);

	duk_enum(ctx, 0, DUK_ENUM_INCLUDE_HIDDEN | DUK_ENUM_NO_PROXY_BEHAVIOR /*flags*/);
	while (duk_next(ctx, 1, 1)) {
		printf("key: %s -> %s\n", duk_safe_to_string(ctx, -2), duk_safe_to_string(ctx, -1));
		duk_pop_2(ctx);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
}
