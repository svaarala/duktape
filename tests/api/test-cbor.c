/*===
*** test_encode (duk_safe_call)
hex_encode(cbor_encode()): a363666f6f636261726471757578830102036362617af5
top after: 0
==> rc=0, result='undefined'
*** test_decode (duk_safe_call)
ToString(cbor_decode()): 1,2,3
top after: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_encode(duk_context *ctx, void *udata) {
	(void) udata;

	duk_eval_string(ctx, "({ foo: 'bar', quux: [ 1, 2, 3 ], baz: true })");
	duk_push_undefined(ctx);
	duk_cbor_encode(ctx, -2, 0);
	duk_pop(ctx);

	duk_hex_encode(ctx, -1);
	printf("hex_encode(cbor_encode()): %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	printf("top after: %ld\n", (long) duk_get_top(ctx));  /* value + dummy */
	duk_set_top(ctx, 0);
	return 0;
}

static duk_ret_t test_decode(duk_context *ctx, void *udata) {
	(void) udata;

	duk_eval_string(ctx, "new Uint8Array([ 0x83, 0x01, 0x02, 0x03 ])");
	duk_push_undefined(ctx);
	duk_cbor_decode(ctx, -2, 0);
	duk_pop(ctx);

	printf("ToString(cbor_decode()): %s\n", duk_to_string(ctx, -1));
	duk_pop(ctx);

	printf("top after: %ld\n", (long) duk_get_top(ctx));  /* value + dummy + get_prop result */
	duk_set_top(ctx, 0);
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_encode);
	TEST_SAFE_CALL(test_decode);
}
