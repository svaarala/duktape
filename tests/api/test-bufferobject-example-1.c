/*
 *  Examples of working with buffer objects from C code.
 */

/*
 *  Example 1: Buffer objects are created by ECMAScript code and then handed
 *  over to a Duktape/C function which accesses the raw bytes from C code.
 */

/*===
*** test_1 (duk_safe_call)
image size: 12x13, width_bytes: 2
...***......
...***......
...***......
..........*.
*****.**..*.
*****.***.*.
*****.*..***
.***.*....*.
..*.**......
...*.**.....
.**...**....
.**...**....
***...***...
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t draw_pixels(duk_context *ctx) {
	unsigned char *ptr;
	duk_size_t sz;
	unsigned char *p;
	long width, height;
	long width_bytes;
	long i, j;

	/* Get data area of buffer argument (plain buffer or any
	 * duk_hbufferobject).
	 *
	 * The returned pointer is stable if the underlying buffer is a
	 * fixed buffer (this is always the case when a buffer object is
	 * created from ECMAScript code e.g. as "new ArrayBuffer()").
	 * For dynamic and external buffers the pointer is stable unless
	 * the buffer is resized or reconfigured.  Caller is responsible
	 * for avoiding the use of stale pointers in such cases.  When in
	 * doubt, relookup the pointer / length right before accessing.
	 *
	 * The duk_{get,require}_buffer_data() calls take into account
	 * "slices" so that the returned ptr/size is always to the active
	 * slice as one would expect compared to how buffers behave in
	 * ECMAScript code.
	 */
	ptr = duk_require_buffer_data(ctx, 0, &sz);

	/* Get width and height.  Buffer contains a 1-bit pixel image,
	 * with rows starting from top represented with ceil(width / 8)
	 * bytes.
	 */
	width = (long) duk_require_int(ctx, 1);
	height = (long) duk_require_int(ctx, 2);
	width_bytes = (width + 7) / 8;
	printf("image size: %ldx%ld, width_bytes: %ld\n",
	       (long) width, (long) height, (long) width_bytes);

	/* Caller must ensure it never accesses beyond the allowed buffer
	 * range which is [0, sz[.  This is critical for memory safety.
	 */
	if (sz < width_bytes * height) {
		return DUK_RET_RANGE_ERROR;
	}

	/* Print pixels.  Actual user code could draw pixels to screen here. */
	p = ptr - 1;  /* dec by one, inner loop advances on first round */
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			unsigned char mask = 1 << (7 - (j & 0x07));
			if ((j & 0x07) == 0) {
				p++;
			}
			if (*p & mask) {
				printf("*");
			} else {
				printf(".");
			}
		}
		printf("\n");
	}

	return 0;
}

static duk_ret_t test_1(duk_context *ctx, void *udata) {
	(void) udata;

	duk_push_c_function(ctx, draw_pixels, 3 /*nargs*/);
	duk_put_global_string(ctx, "drawPixels");

/*
	...***..  ....
	...***..  ....      13 rows
	...***..  ....      12 pixels/row (2 bytes)
	........  ..*.
	*****.**  ..*.
	*****.**  *.*.
	*****.*.  .***
	.***.*..  ..*.
	..*.**..  ....
	...*.**.  ....
	.**...**  ....
	.**...**  ....
	***...**  *...
*/

	duk_eval_string_noresult(ctx,
		"(function () {\n"
		"    var buf = new Buffer(13 * 2);\n"
		"    buf[0] = 0x1c; buf[1] = 0x00;\n"
		"    buf[2] = 0x1c; buf[3] = 0x00;\n"
		"    buf[4] = 0x1c; buf[5] = 0x00;\n"
		"    buf[6] = 0x00; buf[7] = 0x20;\n"
		"    buf[8] = 0xfb; buf[9] = 0x20;\n"
		"    buf[10] = 0xfb; buf[11] = 0xa0;\n"
		"    buf[12] = 0xfa; buf[13] = 0x70;\n"
		"    buf[14] = 0x74; buf[15] = 0x20;\n"
		"    buf[16] = 0x2c; buf[17] = 0x00;\n"
		"    buf[18] = 0x16; buf[19] = 0x00;\n"
		"    buf[20] = 0x63; buf[21] = 0x00;\n"
		"    buf[22] = 0x63; buf[23] = 0x00;\n"
		"    buf[24] = 0xe3; buf[25] = 0x80;\n"
		"    drawPixels(buf, 12 /*width*/, 13 /*height*/);\n"
		"\n"
		"})()\n"
	);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*
 *  Example 2: buffer object is created from C code and used in
 *  ECMAScript code.
 */

/*===
*** test_2 (duk_safe_call)
[object Uint32Array]
8
final top: 1
==> rc=0, result='undefined'
===*/

static duk_ret_t test_2(duk_context *ctx, void *udata) {
	(void) udata;

	duk_eval_string(ctx,
		"(function test(buf) {\n"
		"    print(Object.prototype.toString.call(buf));\n"
		"    print(buf.length);\n"
		"    buf[0] = 0xdeadbeef;\n"
		"})");

	/* Create plain buffer which backs the buffer object. */
	duk_push_fixed_buffer(ctx, 256);

	/* Create a Uint32Array of 8 entries (32 bytes) mapping to
	 * the byte range [16,48[ of the underlying buffer.
	 */
	duk_push_buffer_object(ctx, -1, 16, 32, DUK_BUFOBJ_UINT32ARRAY);  /* offset 16, length 32 bytes, Uint32Array of 8 entries */

	/* The plain buffer is now referenced by the buffer object
	 * and doesn't need to be kept explicitly reachable.
	 */
	duk_remove(ctx, -2);

	/* [ func bufferobject ] */

	duk_call(ctx, 1);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*
 *  Main
 */

void test(duk_context *ctx) {
	TEST_SAFE_CALL(test_1);
	TEST_SAFE_CALL(test_2);
}
