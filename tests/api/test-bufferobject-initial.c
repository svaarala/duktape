/*
 *  Using mixed buffer types from C code in the initial buffer merge before
 *  any C API changes.
 */

/*---
{
    "endianness": "little"
}
---*/

/*===
*** test_to_buffer_1 (duk_safe_call)
0: 66 6f 6f 62 61 72
1: 00 00 00 00 00 00 00 00 00 00
2: 60 61 62 63 64 65 66 67 68 69
3: 60 00 61 00 62 00 63 00 64 00
final top: 8
==> rc=0, result='undefined'
===*/

static duk_ret_t test_to_buffer_1(duk_context *ctx, void *udata) {
	int i, j;
	unsigned char *p;
	duk_size_t sz;

	(void) udata;

	duk_eval_string(ctx, "new Buffer('foobar')");
	/* This is ineffective in Duktape 2.x because ArrayBuffers don't
	 * have virtual index properties anymore (= standard behavior).
	 * So the arraybuffer bytes remain zero.
	 */
	duk_eval_string(ctx,
		"(function () {\n"
		"    var b = new ArrayBuffer(10);\n"
		"    for (var i = 0; i < b.byteLength; i++) { b[i] = 0x40 + i; }\n"
		"    return b;\n"
		"})()\n");
	duk_eval_string(ctx,
		"(function () {\n"
		"    var b = new Uint8Array(new ArrayBuffer(10));\n"
		"    b.set([ 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69 ]);\n"
		"    return b;\n"
		"})()\n");
	/* Host endian specific */
	duk_eval_string(ctx,
		"(function () {\n"
		"    var b = new Uint16Array(new ArrayBuffer(10));\n"
		"    b.set([ 0x60, 0x61, 0x62, 0x63, 0x64 ]);\n"
		"    return b;\n"
		"})()\n");

	for (i = 0; i <= 3; i++) {
		/* Coerce the buffer object into a plain buffer value.
		 * There's no C API support yet to "break down" a buffer
		 * object in pure C.
		 *
		 * The resulting plain buffer is the underlying buffer
		 * of the duk_hbufferobject object, without slice/view
		 * offset information.  For TypedArray views (like the
		 * Uint16Array here) the underlying host bytes are then
		 * available.
		 */

		duk_eval_string(ctx,
			"(function (v) { return Uint8Array.plainOf(v); })");
		duk_dup(ctx, i);
		duk_call(ctx, 1);

		p = (unsigned char *) duk_require_buffer(ctx, -1, &sz);

		printf("%d: ", i);
		for (j = 0; j < sz; j++) {
			if (j > 0) {
				printf(" ");
			}
			printf("%02x", (unsigned int) p[j]);
		}
		printf("\n");
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_to_buffer_1);
}
