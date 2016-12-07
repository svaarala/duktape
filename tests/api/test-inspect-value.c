/*
 *  duk_inspect_value()
 *
 *  The internal implementation for duk_inspect_value() and Duktape.info() is
 *  exactly the same, so just that the C binding minimally works and rely on
 *  Duktape.info() tests for the rest.
 *
 *  One important case to test here is inspecting an out-of-bounds index.
 *  The result should have the type tag DUK_TYPE_NONE.
 */

/*===
*** test_basic (duk_safe_call)
top after inspect: 4
result is object: 1
result.type: 4 (public type number)
final top: 5
==> rc=0, result='undefined'
*** test_buffer_variants (duk_safe_call)
[7,0]
[7,1]
[7,2]
final top: 0
==> rc=0, result='undefined'
*** test_invalid_index1 (duk_safe_call)
top after inspect: 2
result is object: 1
result.type: 0 (public type number)
final top: 3
==> rc=0, result='undefined'
*** test_invalid_index2 (duk_safe_call)
top after inspect: 1
result is object: 1
result.type: 0 (public type number)
final top: 2
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_string(ctx, "dummy");
	duk_push_number(ctx, 123.4);
	duk_push_string(ctx, "dummy");

	duk_inspect_value(ctx, -2);

	printf("top after inspect: %ld\n", (long) duk_get_top(ctx));

	printf("result is object: %ld\n", (long) duk_is_object(ctx, -1));

	duk_get_prop_string(ctx, -1, "type");
	printf("result.type: %ld (public type number)\n", (long) duk_to_int(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_buffer_variants(duk_context *ctx, void *udata) {
	unsigned char buf[16];

	(void) udata;

	duk_eval_string(ctx,
		"(function (b1, b2, b3) {\n"
		"    [ b1, b2, b3 ].forEach(function (b) {\n"
		"        var info = Duktape.info(b);\n"
		"        var t = [ info.type, info.variant ];\n"
		"        print(Duktape.enc('jx', t));\n"
		"    });\n"
		"})");
	duk_push_fixed_buffer(ctx, 1024);
	duk_push_dynamic_buffer(ctx, 256);
	duk_push_external_buffer(ctx);
	duk_config_buffer(ctx, -1, (void *) buf, 16);
	duk_call(ctx, 3 /*nargs*/);
	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_invalid_index1(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_string(ctx, "dummy");

	duk_inspect_value(ctx, -2);

	printf("top after inspect: %ld\n", (long) duk_get_top(ctx));

	printf("result is object: %ld\n", (long) duk_is_object(ctx, -1));

	duk_get_prop_string(ctx, -1, "type");
	printf("result.type: %ld (public type number)\n", (long) duk_to_int(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_invalid_index2(duk_context *ctx, void *udata) {
	(void) udata;

	duk_inspect_value(ctx, DUK_INVALID_INDEX);

	printf("top after inspect: %ld\n", (long) duk_get_top(ctx));

	printf("result is object: %ld\n", (long) duk_is_object(ctx, -1));

	duk_get_prop_string(ctx, -1, "type");
	printf("result.type: %ld (public type number)\n", (long) duk_to_int(ctx, -1));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_buffer_variants);
	TEST_SAFE_CALL(test_invalid_index1);
	TEST_SAFE_CALL(test_invalid_index2);
}
