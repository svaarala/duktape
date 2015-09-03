/*
 *  When doing a TypedArray.set() with compatible views, Duktape tries to
 *  use a fast copy (memmove) when possible.
 *
 *  The set() operation must work correctly when the underlying buffer is
 *  the same (source overlaps target).  With an external buffer value it
 *  is possible for the source and target to be separate buffer values which
 *  nevertheless point to the same underlying memory area, so that the check
 *  must be made using pointers to the final source and destination.
 *
 *  Test that this case also works correctly.
 */

/*===
*** test_basic_overlap (duk_safe_call)
offset: 0
|00112233445566778899aabbccddeeff|
|0011223344556677|
|001122330011223344556677ccddeeff|
|0011223300112233|
offset: 1
|00112233445566778899aabbccddeeff|
|1122334455667788|
|001122331122334455667788ccddeeff|
|1122331122334455|
offset: 2
|00112233445566778899aabbccddeeff|
|2233445566778899|
|001122332233445566778899ccddeeff|
|2233223344556677|
offset: 3
|00112233445566778899aabbccddeeff|
|33445566778899aa|
|0011223333445566778899aaccddeeff|
|3333445566778899|
offset: 4
|00112233445566778899aabbccddeeff|
|445566778899aabb|
|00112233445566778899aabbccddeeff|
|445566778899aabb|
offset: 5
|00112233445566778899aabbccddeeff|
|5566778899aabbcc|
|001122335566778899aabbccccddeeff|
|66778899aabbcccc|
offset: 6
|00112233445566778899aabbccddeeff|
|66778899aabbccdd|
|0011223366778899aabbccddccddeeff|
|8899aabbccddccdd|
offset: 7
|00112233445566778899aabbccddeeff|
|778899aabbccddee|
|00112233778899aabbccddeeccddeeff|
|aabbccddeeccddee|
offset: 8
|00112233445566778899aabbccddeeff|
|8899aabbccddeeff|
|001122338899aabbccddeeffccddeeff|
|ccddeeffccddeeff|
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic_overlap(duk_context *ctx) {
	unsigned char buf[16];
	int offset;
	int i;

	for (offset = 0; offset <= 8; offset++) {
		printf("offset: %d\n", offset);

		/* 001122334455...ff */
		for (i = 0; i < sizeof(buf); i++) {
			buf[i] = (unsigned char) (0x11 * i);
		}

		/* Create two separate external buffer values that point to the
		 * same underlying C array with some overlap.
		 */
		duk_eval_string(ctx,
			"(function (plain1, plain2) {\n"
			"    var b1 = new Uint8Array(new ArrayBuffer(plain1));\n"
			"    var b2 = new Uint8Array(new ArrayBuffer(plain2));\n"
			"    print(Duktape.enc('jx', b1));\n"
			"    print(Duktape.enc('jx', b2));\n"
			"    b1.set(b2, 4);\n"
			"    print(Duktape.enc('jx', b1));\n"
			"    print(Duktape.enc('jx', b2));\n"
			"})");
		duk_push_external_buffer(ctx);
		duk_config_buffer(ctx, -1, (void *) buf, 16);  /* [0,16[ */
		duk_push_external_buffer(ctx);
		duk_config_buffer(ctx, -1, (void *) (buf + offset), 8);
		duk_call(ctx, 2 /*nargs*/);
		duk_pop(ctx);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_expand_overlap (duk_safe_call)
offset: 0
|04050607|
|000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f|
|04000000|
|00010203040000000500000006000000070000001415161718191a1b1c1d1e1f|
00010203040000000500000006000000070000001415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f
offset: 1
|04050607|
|0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20|
|04040000|
|010203040400000005000000060000000700000015161718191a1b1c1d1e1f20|
00010203040400000005000000060000000700000015161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f
offset: 2
|04050607|
|02030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f2021|
|04050400|
|0203040504000000050000000600000007000000161718191a1b1c1d1e1f2021|
00010203040504000000050000000600000007000000161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f
offset: 3
|04050607|
|030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122|
|04050604|
|03040506040000000500000006000000070000001718191a1b1c1d1e1f202122|
00010203040506040000000500000006000000070000001718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f
offset: 4
|04050607|
|0405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20212223|
|04050607|
|040506070400000005000000060000000700000018191a1b1c1d1e1f20212223|
00010203040506070400000005000000060000000700000018191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f
offset: 5
|04050607|
|05060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f2021222324|
|04050607|
|0506070804000000050000000600000007000000191a1b1c1d1e1f2021222324|
00010203040506070804000000050000000600000007000000191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f
offset: 6
|04050607|
|060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425|
|04050607|
|06070809040000000500000006000000070000001a1b1c1d1e1f202122232425|
00010203040506070809040000000500000006000000070000001a1b1c1d1e1f202122232425262728292a2b2c2d2e2f
offset: 7
|04050607|
|0708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20212223242526|
|04050607|
|0708090a040000000500000006000000070000001b1c1d1e1f20212223242526|
000102030405060708090a040000000500000006000000070000001b1c1d1e1f202122232425262728292a2b2c2d2e2f
offset: 8
|04050607|
|08090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f2021222324252627|
|04050607|
|08090a0b040000000500000006000000070000001c1d1e1f2021222324252627|
000102030405060708090a0b040000000500000006000000070000001c1d1e1f202122232425262728292a2b2c2d2e2f
offset: 9
|04050607|
|090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728|
|04050607|
|090a0b0c040000000500000006000000070000001d1e1f202122232425262728|
000102030405060708090a0b0c040000000500000006000000070000001d1e1f202122232425262728292a2b2c2d2e2f
offset: 10
|04050607|
|0a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20212223242526272829|
|04050607|
|0a0b0c0d040000000500000006000000070000001e1f20212223242526272829|
000102030405060708090a0b0c0d040000000500000006000000070000001e1f202122232425262728292a2b2c2d2e2f
offset: 11
|04050607|
|0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a|
|04050607|
|0b0c0d0e040000000500000006000000070000001f202122232425262728292a|
000102030405060708090a0b0c0d0e040000000500000006000000070000001f202122232425262728292a2b2c2d2e2f
offset: 12
|04050607|
|0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b|
|04050607|
|0c0d0e0f04000000050000000600000007000000202122232425262728292a2b|
000102030405060708090a0b0c0d0e0f04000000050000000600000007000000202122232425262728292a2b2c2d2e2f
offset: 13
|04050607|
|0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c|
|04050607|
|0d0e0f10040000000500000006000000070000002122232425262728292a2b2c|
000102030405060708090a0b0c0d0e0f10040000000500000006000000070000002122232425262728292a2b2c2d2e2f
offset: 14
|04050607|
|0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d|
|04050607|
|0e0f10110400000005000000060000000700000022232425262728292a2b2c2d|
000102030405060708090a0b0c0d0e0f10110400000005000000060000000700000022232425262728292a2b2c2d2e2f
offset: 15
|04050607|
|0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e|
|04050607|
|0f10111204000000050000000600000007000000232425262728292a2b2c2d2e|
000102030405060708090a0b0c0d0e0f10111204000000050000000600000007000000232425262728292a2b2c2d2e2f
final top: 0
==> rc=0, result='undefined'
===*/

/* Test the case where source is a Uint8Array which is .set() to a Uint32Array
 * which means that every byte in the input is expanded to 4 bytes in the
* output.  In this case it's possible for the output to overlap the input
 * in a way that neither forward or backward copying directive is not enough
 * but a genuine temporary is needed.  Illustration:
 *
 *   SRC        |01020304|
 *               | | | |
 *               | | | `--------------------.
 *               | | `--------------.       |
 *               | `--------.       |       |
 *               `--.       |       |       |
 *                  v       v       v       v
 *                  .------..------..------..------.
 *                  |      ||      ||      ||      |
 *   DST    XXYYXXYY01000000020000000300000004000000XXYYXXYY (little endian)
 *
 *  The underlying duk_hbuffers for SRC and DST are separate external buffer
 *  values, but they back to the same area which requires a pointer based
 *  check in the implementation.
 */

static duk_ret_t test_expand_overlap(duk_context *ctx) {
	unsigned char buf[48];
	int offset;
	int i;

	for (offset = 0; offset < 16; offset++) {  /* dst offset */
		printf("offset: %d\n", offset);

		for (i = 0; i < 48; i++) {
			buf[i] = (unsigned char) i;
		}

		duk_push_external_buffer(ctx);  /* src */
		duk_config_buffer(ctx, -1, (void *) (buf + 4), 4);  /* |04050607| */

		duk_push_external_buffer(ctx);  /* dst */
		duk_config_buffer(ctx, -1, (void *) (buf + offset), 32);

		duk_eval_string(ctx,
			"(function (plain_src, plain_dst) {\n"
			"    var bsrc = new Uint8Array(new ArrayBuffer(plain_src));\n"
			"    var bdst = new Uint32Array(new ArrayBuffer(plain_dst));\n"
			"    print(Duktape.enc('jx', bsrc));\n"
			"    print(Duktape.enc('jx', bdst));\n"
			"    bdst.set(bsrc, 1);\n"
			"    print(Duktape.enc('jx', bsrc));\n"
			"    print(Duktape.enc('jx', bdst));\n"
			"})");
		duk_dup(ctx, 0);
		duk_dup(ctx, 1);
		duk_call(ctx, 2);
		duk_pop_n(ctx, 3);

		for (i = 0; i < 48; i++) {
			printf("%02x", (unsigned int) buf[i]);
		}
		printf("\n");
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_basic_overlap);
	TEST_SAFE_CALL(test_expand_overlap);
	(void) ctx;
}
