/*===
*** test_encode (duk_safe_call)
json encode: {"foo":123,"bar":"quux"}
top after: 2
==> rc=0, result='undefined'
*** test_encode_apidoc (duk_safe_call)
JSON encoded: {"meaningOfLife":42}
top after: 0
==> rc=0, result='undefined'
*** test_decode (duk_safe_call)
json decode, result.foo=bar
top after: 3
==> rc=0, result='undefined'
*** test_decode_apidoc (duk_safe_call)
JSON decoded meaningOfLife is: 42
top after: 0
==> rc=0, result='undefined'
*** test_decode_error (duk_safe_call)
ret: 1
SyntaxError: invalid json (at offset N)
top after: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_encode(duk_context *ctx) {
	duk_push_object(ctx);
	duk_push_int(ctx, 123);
	duk_put_prop_string(ctx, -2, "foo");
	duk_push_string(ctx, "quux");
	duk_put_prop_string(ctx, -2, "bar");
	duk_push_int(ctx, 123);  /* dummy */
	printf("json encode: %s\n", duk_json_encode(ctx, -2));
	printf("top after: %ld\n", (long) duk_get_top(ctx));  /* value + dummy */
	duk_set_top(ctx, 0);
	return 0;
}

static duk_ret_t test_encode_apidoc(duk_context *ctx) {
	duk_push_object(ctx);
	duk_push_int(ctx, 42);
	duk_put_prop_string(ctx, -2, "meaningOfLife");
	printf("JSON encoded: %s\n", duk_json_encode(ctx, -1));
	duk_pop(ctx);
	printf("top after: %ld\n", (long) duk_get_top(ctx));  /* value + dummy + get_prop result */
	duk_set_top(ctx, 0);
	return 0;
}

static duk_ret_t test_decode(duk_context *ctx) {
	duk_push_string(ctx, "{\"foo\":\"bar\"}");
	duk_push_int(ctx, 321);  /* dummy */
	duk_json_decode(ctx, -2);
	duk_get_prop_string(ctx, -2, "foo");
	printf("json decode, result.foo=%s\n", duk_get_string(ctx, -1));
	printf("top after: %ld\n", (long) duk_get_top(ctx));  /* value + dummy + get_prop result */
	duk_set_top(ctx, 0);
	return 0;
}

static duk_ret_t test_decode_apidoc(duk_context *ctx) {
	duk_push_string(ctx, "{\"meaningOfLife\":42}");
	duk_json_decode(ctx, -1);
	duk_get_prop_string(ctx, -1, "meaningOfLife");
	printf("JSON decoded meaningOfLife is: %s\n", duk_to_string(ctx, -1));
	duk_pop_2(ctx);
	printf("top after: %ld\n", (long) duk_get_top(ctx));  /* value + dummy + get_prop result */
	duk_set_top(ctx, 0);
	return 0;
}

static duk_ret_t test_decode_error_raw(duk_context *ctx) {
	duk_json_decode(ctx, -1);
	return 1;
}

static duk_ret_t test_decode_error(duk_context *ctx) {
	/* The JSON.parse() error message includes a byte offset, so we need to
	 * normalize it.
	 */
	duk_int_t ret;

	duk_push_string(ctx, "{\"meaningOfLife\":,42}");
	ret = duk_safe_call(ctx, test_decode_error_raw, 1 /*nargs*/, 1 /*nrets*/);
	printf("ret: %ld\n", (long) ret);

	duk_eval_string(ctx, "(function (x) { print(x.replace(/at offset \\d+/, 'at offset N')); })");
	duk_dup(ctx, -2);
	duk_to_string(ctx, -1);
	duk_call(ctx, 1 /*nargs*/);

	duk_pop(ctx);  /* duk_call result */
	duk_pop(ctx);  /* duk_safe_call result */

	printf("top after: %ld\n", (long) duk_get_top(ctx));
	duk_set_top(ctx, 0);
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_encode);
	TEST_SAFE_CALL(test_encode_apidoc);
	TEST_SAFE_CALL(test_decode);
	TEST_SAFE_CALL(test_decode_apidoc);
	TEST_SAFE_CALL(test_decode_error);
}
