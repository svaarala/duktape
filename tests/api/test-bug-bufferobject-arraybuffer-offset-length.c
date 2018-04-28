/*===
*** test_basic (duk_safe_call)
uint8array index 10: 20
arraybuffer index 10: 10
arraybuffer index byteOffset + 10: 20
uint8array index 10: 20
arraybuffer index 10: 10
arraybuffer index byteOffset + 10: 20
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx, void *udata) {
	unsigned char *p;
	int i;

	(void) udata;

	p = (unsigned char *) duk_push_fixed_buffer(ctx, 128);
	for (i = 0; i < 128; i++) {
		p[i] = (unsigned char) i;
	}

	duk_push_buffer_object(ctx, -1, 10, 40, DUK_BUFOBJ_UINT8ARRAY);

	/* Up to Duktape 1.6.x duk_push_buffer_object() creates an Uint8Array
	 * with a certain offset/length, and uses that same offset/length for
	 * the underlying ArrayBuffer.
	 *
	 * This makes the ArrayBuffer offsetted (offset != 0).  It also means
	 * that accessing the ArrayBuffer at index .byteOffset + N does -not-
	 * match Uint8Array at offset N as one would normally expect.  Instead
	 * the .byteOffset is already account for in the ArrayBuffer internally.
	 *
	 * Duktape 1.7.x and 2.0.x changes this so that the .byteOffset is 0,
	 * and the ArrayBuffer .byteLength is set to .byteOffset + .byteLength
	 * so that the entire constructed view is visible also in the ArrayBuffer.
	 */

	duk_eval_string(ctx,
		"(function (buf) {\n"
		"    var arrbuf = buf.buffer;\n"
		"    print('uint8array index 10:', buf[10]);\n"
		"    print('arraybuffer index 10:', new Uint8Array(arrbuf)[10]);\n"
		"    var byteOffset = buf.byteOffset;\n"
		"    print('arraybuffer index byteOffset + 10:', new Uint8Array(arrbuf)[byteOffset + 10]);\n"
		"})");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);

	duk_pop(ctx);  /* eval result */
	duk_pop_2(ctx);  /* fixed buffer, buffer object */

	/* ECMAScript equivalent. */

	duk_eval_string_noresult(ctx,
		"(function (buf) {\n"
	        "    var arrbuf = new ArrayBuffer(50);\n"
		"    var u8 = new Uint8Array(arrbuf);  // for init\n"
		"    for (var i = 0; i < 50; i++) { u8[i] = i; }\n"
		"    var buf = new Uint8Array(arrbuf, 10, 40);\n"
		"    print('uint8array index 10:', buf[10]);\n"
		"    print('arraybuffer index 10:', new Uint8Array(arrbuf)[10]);\n"
		"    var byteOffset = buf.byteOffset;\n"
		"    print('arraybuffer index byteOffset + 10:', new Uint8Array(arrbuf)[byteOffset + 10]);\n"
		"})()");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic);
}
