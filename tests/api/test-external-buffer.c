/*
 *  External buffer type
 */

/*===
*** test_1 (duk_safe_call)
external buffer, pointer is NULL: 1, length: 0
external buffer, pointer is NULL: 0, length: 236
buf before: aa aa 55 55 55 55 55 55 aa aa aa aa aa aa 55 55 55 55 55 55 aa aa aa aa aa aa 55 55 55 55 55 55 aa aa aa aa aa aa 55 55 55 55 55 55 aa aa aa aa aa aa 55 55 55 55 55 55 aa aa aa aa aa aa 55 55 55 55 55 55 aa aa aa aa aa aa 55 55 55 55 55 55 aa aa aa aa aa aa 55 55 55 55 55 55 aa aa aa aa aa aa 55 55 55 55 55 55 aa aa aa aa aa aa 55 55 55 55 55 55 aa aa aa aa aa aa 55 55 55 55 55 55 aa aa aa aa aa aa 55 55 55 55 55 55 aa aa aa aa aa aa 55 55 55 55 55 55 aa aa aa aa aa aa 55 55 55 55 55 55 aa aa aa aa aa aa 55 55 55 55 55 55 aa aa aa aa aa aa 55 55 55 55 55 55 aa aa aa aa aa aa 55 55 55 55 55 55 aa aa aa aa aa aa 55 55 55 55 55 55 aa aa aa aa aa aa 55 55 55 55 55 55 aa aa aa aa aa aa 55 55 55 55 55 55
global_buffer: 55 55 55 55 55 55 aa aa aa aa 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f 20 21 22 23 24 25 26 27 28 29 2a 2b 2c 2d 2e 2f 30 31 32 33 34 35 36 37 38 39 3a 3b 3c 3d 3e 3f 40 41 42 43 44 45 46 47 48 49 4a 4b 4c 4d 4e 4f 50 51 52 53 54 55 56 57 58 59 5a 5b 5c 5d 5e 5f 60 61 62 63 64 65 66 67 68 69 6a 6b 6c 6d 6e 6f 70 71 72 73 74 75 76 77 78 79 7a 7b 7c 7d 7e 7f 80 81 82 83 84 85 86 87 88 89 8a 8b 8c 8d 8e 8f 90 91 92 93 94 95 96 97 98 99 9a 9b 9c 9d 9e 9f a0 a1 a2 a3 a4 a5 a6 a7 a8 a9 aa ab ac ad ae af b0 b1 b2 b3 b4 b5 b6 b7 b8 b9 ba bb bc bd be bf c0 c1 c2 c3 c4 c5 c6 c7 c8 c9 ca cb cc cd ce cf d0 d1 d2 d3 d4 d5 d6 d7 d8 d9 da db dc dd de df e0 e1 e2 e3 e4 e5 e6 e7 e8 e9 ea eb aa aa aa aa aa aa 55 55 55 55
final top: 1
==> rc=0, result='undefined'
*** test_2a (duk_safe_call)
==> rc=1, result='TypeError: wrong buffer type'
*** test_2b (duk_safe_call)
==> rc=1, result='TypeError: wrong buffer type'
*** test_3 (duk_safe_call)
==> rc=1, result='TypeError: buffer required, found [object Object] (stack index -1)'
*** test_4 (duk_safe_call)
["censoredtag","censoredpointer","censoredrefcount","censoredheapsize"]
["censoredtag","censoredpointer","censoredrefcount","censoredheapsize",256]
["censoredtag","censoredpointer","censoredrefcount","censoredheapsize",16]
final top: 0
==> rc=0, result='undefined'
===*/

unsigned char global_buffer[256];

static duk_ret_t test_1(duk_context *ctx) {
	void *ptr;
	duk_size_t len;
	int i;

	for (i = 0; i < sizeof(global_buffer); i++) {
		if ((i / 3) & 2) {
			global_buffer[i] = 0xaa;
		} else {
			global_buffer[i] = 0x55;
		}
	}

	duk_set_top(ctx, 0);

	duk_push_external_buffer(ctx);
	ptr = duk_require_buffer(ctx, -1, &len);
	printf("external buffer, pointer is NULL: %d, length: %ld\n",
	       (ptr == NULL) ? 1 : 0, (long) len);

	/* Create an external buffer mapping to global_buffer, with 10 bytes
	 * from start and end outside of the external buffer.  Duktape should
	 * never write there.
	 */
	duk_config_buffer(ctx, -1, (void *) (global_buffer + 10), (duk_size_t) sizeof(global_buffer) - 20);
	ptr = duk_require_buffer(ctx, -1, &len);
	printf("external buffer, pointer is NULL: %d, length: %ld\n",
	       (ptr == NULL) ? 1 : 0, (long) len);

	/* [ buf ] */

	/* Overwrite buffer intentionally: this is caught by Duktape and the
	 * writes are ignored.
	 */
	duk_eval_string(ctx,
		"(function test(buf) {\n"
		"    var tmp = [];\n"
		"    for (var i = 0; i < buf.length ; i++) { tmp[i] = buf[i].toString(16); }\n"
		"    print('buf before: ' + tmp.join(' '));\n"
		"    for (var i = -1000; i < buf.length + 1000; i++) { buf[i] = i; }\n"
		"})");
	duk_dup(ctx, 0);
	duk_call(ctx, 1);  /* test(buf) */
	duk_pop(ctx);

	printf("global_buffer:");
	for (i = 0; i < sizeof(global_buffer); i++) {
		printf(" %02x", (int) global_buffer[i]);
	}
	printf("\n");

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Attempt to set external buffer for wrong buffer type. */
static duk_ret_t test_2a(duk_context *ctx) {
	duk_push_fixed_buffer(ctx, 1024);
	duk_config_buffer(ctx, -1, (void *) global_buffer, sizeof(global_buffer));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_2b(duk_context *ctx) {
	duk_push_dynamic_buffer(ctx, 1024);
	duk_config_buffer(ctx, -1, (void *) global_buffer, sizeof(global_buffer));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Attempt to set external buffer for wrong type altogether. */
static duk_ret_t test_3(duk_context *ctx) {
	duk_push_object(ctx);
	duk_config_buffer(ctx, -1, (void *) global_buffer, sizeof(global_buffer));

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Check Duktape.info() for external buffers. */
static duk_ret_t test_4(duk_context *ctx) {
	unsigned char buf[16];

	/* Censor refcount too because e.g. in shuffle torture test the
	 * refcount will be different than otherwise.
	 */
	duk_eval_string(ctx,
		"(function (b1, b2, b3) {\n"
		"    [ b1, b2, b3 ].forEach(function (b) {\n"
		"        var t = Duktape.info(b);\n"
		"        t[0] = 'censoredtag';\n"
		"        t[1] = 'censoredpointer';\n"
		"        t[2] = 'censoredrefcount';\n"
		"        t[3] = 'censoredheapsize';\n"
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

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2a);
	TEST_SAFE_CALL(test_2b);
	TEST_SAFE_CALL(test_3);
	TEST_SAFE_CALL(test_4);
}
