/*
 *  duk_push_buffer_object() API call
 */

static void register_dump_buffer_info(duk_context *ctx) {
	duk_eval_string_noresult(ctx,
		"function dumpBufferInfo (v) {\n"
		"    var p = Object.getPrototypeOf(v);\n"
		"    var instof = [];\n"
		"    var prot = [];\n"

		"    if (v instanceof Duktape.Buffer) { instof.push('Duktape.Buffer'); }\n"
		"    if (v instanceof Buffer) { instof.push('Buffer'); }\n"
		"    if (v instanceof ArrayBuffer) { instof.push('ArrayBuffer'); }\n"
		"    if (v instanceof DataView) { instof.push('DataView'); }\n"
		"    if (v instanceof Int8Array) { instof.push('Int8Array'); }\n"
		"    if (v instanceof Uint8Array) { instof.push('Uint8Array'); }\n"
		"    if (v instanceof Uint8ClampedArray) { instof.push('Uint8ClampedArray'); }\n"
		"    if (v instanceof Int16Array) { instof.push('Int16Array'); }\n"
		"    if (v instanceof Uint16Array) { instof.push('Uint16Array'); }\n"
		"    if (v instanceof Int32Array) { instof.push('Int32Array'); }\n"
		"    if (v instanceof Uint32Array) { instof.push('Uint32Array'); }\n"
		"    if (v instanceof Float32Array) { instof.push('Float32Array'); }\n"
		"    if (v instanceof Float64Array) { instof.push('Float64Array'); }\n"

		"    if (p === Duktape.Buffer.prototype) { prot.push('Duktape.Buffer.prototype'); }\n"
		"    if (p === Buffer.prototype) { prot.push('Buffer.prototype'); }\n"
		"    if (p === ArrayBuffer.prototype) { prot.push('ArrayBuffer.prototype'); }\n"
		"    if (p === DataView.prototype) { prot.push('DataView.prototype'); }\n"
		"    if (p === Int8Array.prototype) { prot.push('Int8Array.prototype'); }\n"
		"    if (p === Uint8Array.prototype) { prot.push('Uint8Array.prototype'); }\n"
		"    if (p === Uint8ClampedArray.prototype) { prot.push('Uint8ClampedArray.prototype'); }\n"
		"    if (p === Int16Array.prototype) { prot.push('Int16Array.prototype'); }\n"
		"    if (p === Uint16Array.prototype) { prot.push('Uint16Array.prototype'); }\n"
		"    if (p === Int32Array.prototype) { prot.push('Int32Array.prototype'); }\n"
		"    if (p === Uint32Array.prototype) { prot.push('Uint32Array.prototype'); }\n"
		"    if (p === Float32Array.prototype) { prot.push('Float32Array.prototype'); }\n"
		"    if (p === Float64Array.prototype) { prot.push('Float64Array.prototype'); }\n"

		"    print(typeof v, Object.prototype.toString.call(v), v.length, v.byteOffset, v.byteLength, v.BYTES_PER_ELEMENT, typeof v.buffer);\n"
		"    print(v instanceof Duktape.Buffer, v instanceof Buffer, v instanceof ArrayBuffer, v instanceof DataView, v instanceof Int8Array, v instanceof Uint8Array, v instanceof Uint8ClampedArray, v instanceof Int16Array, v instanceof Uint16Array, v instanceof Int32Array, v instanceof Uint32Array, v instanceof Float32Array, v instanceof Float64Array, '->', instof.join(','));\n"
		"    print(p === Duktape.Buffer.prototype, p === Buffer.prototype, p === ArrayBuffer.prototype, p === DataView.prototype, p === Int8Array.prototype, p === Uint8Array.prototype, p === Uint8ClampedArray.prototype, p === Int16Array.prototype, p === Uint16Array.prototype, p === Int32Array.prototype, p === Uint32Array.prototype, p === Float32Array.prototype, p === Float64Array.prototype, '->', prot.join(','));\n"
		"}");
}

/*===
*** test_basic (duk_safe_call)
object [object Buffer] 32 128 32 1 undefined
true false false false false false false false false false false false false -> Duktape.Buffer
true false false false false false false false false false false false false -> Duktape.Buffer.prototype
object [object Buffer] 32 128 32 1 undefined
false true false false false false false false false false false false false -> Buffer
false true false false false false false false false false false false false -> Buffer.prototype
object [object ArrayBuffer] 32 128 32 1 undefined
false false true false false false false false false false false false false -> ArrayBuffer
false false true false false false false false false false false false false -> ArrayBuffer.prototype
object [object DataView] 32 128 32 1 object
false false false true false false false false false false false false false -> DataView
false false false true false false false false false false false false false -> DataView.prototype
object [object Int8Array] 32 128 32 1 object
false false false false true false false false false false false false false -> Int8Array
false false false false true false false false false false false false false -> Int8Array.prototype
object [object Uint8Array] 32 128 32 1 object
false false false false false true false false false false false false false -> Uint8Array
false false false false false true false false false false false false false -> Uint8Array.prototype
object [object Uint8ClampedArray] 32 128 32 1 object
false false false false false false true false false false false false false -> Uint8ClampedArray
false false false false false false true false false false false false false -> Uint8ClampedArray.prototype
object [object Int16Array] 16 128 32 2 object
false false false false false false false true false false false false false -> Int16Array
false false false false false false false true false false false false false -> Int16Array.prototype
object [object Uint16Array] 16 128 32 2 object
false false false false false false false false true false false false false -> Uint16Array
false false false false false false false false true false false false false -> Uint16Array.prototype
object [object Int32Array] 8 128 32 4 object
false false false false false false false false false true false false false -> Int32Array
false false false false false false false false false true false false false -> Int32Array.prototype
object [object Uint32Array] 8 128 32 4 object
false false false false false false false false false false true false false -> Uint32Array
false false false false false false false false false false true false false -> Uint32Array.prototype
object [object Float32Array] 8 128 32 4 object
false false false false false false false false false false false true false -> Float32Array
false false false false false false false false false false false true false -> Float32Array.prototype
object [object Float64Array] 4 128 32 8 object
false false false false false false false false false false false false true -> Float64Array
false false false false false false false false false false false false true -> Float64Array.prototype
final top: 0
==> rc=0, result='undefined'
===*/

static duk_ret_t test_basic(duk_context *ctx) {
	duk_uint_t test[] = {
		DUK_BUFOBJ_DUKTAPE_BUFFER,
		DUK_BUFOBJ_NODEJS_BUFFER,
		DUK_BUFOBJ_ARRAYBUFFER,
		DUK_BUFOBJ_DATAVIEW,
		DUK_BUFOBJ_INT8ARRAY,
		DUK_BUFOBJ_UINT8ARRAY,
		DUK_BUFOBJ_UINT8CLAMPEDARRAY,
		DUK_BUFOBJ_INT16ARRAY,
		DUK_BUFOBJ_UINT16ARRAY,
		DUK_BUFOBJ_INT32ARRAY,
		DUK_BUFOBJ_UINT32ARRAY,
		DUK_BUFOBJ_FLOAT32ARRAY,
		DUK_BUFOBJ_FLOAT64ARRAY,
	};
	int i;
	unsigned char extbuf[256];

	for (i = 0; i < sizeof(test) / sizeof(duk_uint_t); i++) {
		switch (i % 3) {
		case 0:
			duk_push_fixed_buffer(ctx, 256);
			break;
		case 1:
			duk_push_dynamic_buffer(ctx, 256);
			break;
		case 2:
			duk_push_external_buffer(ctx);
			duk_config_buffer(ctx, -1, (void *) extbuf, 256);
			break;
		}

		duk_push_undefined(ctx);  /* dummy */

		duk_push_buffer_object(ctx, -2, 128, 32, test[i]);

		duk_eval_string(ctx, "dumpBufferInfo");
		duk_dup(ctx, -2);
		duk_call(ctx, 1);

		/* ... plain undefined bufferobject result */
		duk_pop_n(ctx, 4);
	}

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_view_buffer_prop (duk_safe_call)
[object Uint8Array]
object [object Uint8Array] 22 16 22 1 object
false false false false false true false false false false false false false -> Uint8Array
false false false false false true false false false false false false false -> Uint8Array.prototype
22
16
22
1
[object ArrayBuffer]
[object ArrayBuffer] false false false
[object ArrayBuffer]
object [object ArrayBuffer] 22 16 22 1 undefined
false false true false false false false false false false false false false -> ArrayBuffer
false false true false false false false false false false false false false -> ArrayBuffer.prototype
22
16
22
1
undefined
123 123
extbuf[16 + 3] = 123
final top: 1
==> rc=0, result='undefined'
===*/

/* The basic test ensures all typed array views get a .buffer property.
 * Test the .buffer reference in more detail: check that property
 * attributes are correct, check that it backs to the same slice, etc.
 */
static duk_ret_t test_view_buffer_prop(duk_context *ctx) {
	unsigned char extbuf[256];

	duk_push_external_buffer(ctx);
	duk_config_buffer(ctx, -1, (void *) extbuf, 256);
	duk_push_buffer_object(ctx, -1, 16, 22, DUK_BUFOBJ_UINT8ARRAY);

	duk_eval_string(ctx,
		"(function (v) {\n"
		"    var pd;\n"
		"    print(Object.prototype.toString.call(v));\n"
		"    dumpBufferInfo(v);\n"
		"    print(v.length);\n"
		"    print(v.byteOffset);\n"
		"    print(v.byteLength);\n"
		"    print(v.BYTES_PER_ELEMENT);\n"
		"    print(v.buffer);\n"
		"    pd = Object.getOwnPropertyDescriptor(v, 'buffer');\n"
		"    print(pd.value, pd.writable, pd.enumerable, pd.configurable);\n"
		"    print(Object.prototype.toString.call(v.buffer));\n"
		"    dumpBufferInfo(v.buffer);\n"
		"    print(v.buffer.length);\n"  /* some of these are Duktape custom */
		"    print(v.buffer.byteOffset);\n"
		"    print(v.buffer.byteLength);\n"
		"    print(v.buffer.BYTES_PER_ELEMENT);\n"
		"    print(v.buffer.buffer);\n"
		"    v[3] = 123;  /* check that backing buffer and slice matches */\n"
		"    print(v[3], new Uint8Array(v.buffer)[3]);\n"
		"})");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);
	duk_pop_2(ctx);

	printf("extbuf[16 + 3] = %d\n", (int) extbuf[16 + 3]);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_unaligned (duk_safe_call)
object [object Uint32Array] 16 7 64 4 object
false false false false false false false false false false true false false -> Uint32Array
false false false false false false false false false false true false false -> Uint32Array.prototype
final top: 0
==> rc=0, result='undefined'
===*/

/* Normally typed array constructors require that the slice begin at a multiple
 * of the element size (e.g. 4 bytes for Uint32Array) and that the slice ends
 * evenly.  The C API doesn't pose such restrictions.
 */
static duk_ret_t test_unaligned(duk_context *ctx) {
	duk_push_fixed_buffer(ctx, 256);
	duk_push_buffer_object(ctx, -1, 7, 64, DUK_BUFOBJ_UINT32ARRAY);

	duk_eval_string(ctx, "dumpBufferInfo");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_pop_n(ctx, 2);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_unaligned_uneven (duk_safe_call)
object [object Uint32Array] 15 7 63 4 object
false false false false false false false false false false true false false -> Uint32Array
false false false false false false false false false false true false false -> Uint32Array.prototype
final top: 0
==> rc=0, result='undefined'
===*/

/* Normally typed array constructors require that the slice begin at a multiple
 * of the element size (e.g. 4 bytes for Uint32Array) and that the slice ends
 * evenly.  The C API doesn't pose such restrictions; the .length (number of
 * elements) will become the number of full elements allowed by the slice.
 * The .byteOffset and .byteLength will reflect the call arguments, e.g.
 * .byteLength is 63 here, which means that .length x .BYTES_PER_ELEMENT
 * won't match .byteLength in this case.
 */
static duk_ret_t test_unaligned_uneven(duk_context *ctx) {
	/* Start at an unaligned offset, with slice length NOT a multiple
	 * of 4.
	 */
	duk_push_fixed_buffer(ctx, 256);
	duk_push_buffer_object(ctx, -1, 7, 63, DUK_BUFOBJ_UINT32ARRAY);

	duk_eval_string(ctx, "dumpBufferInfo");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_pop_n(ctx, 2);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_uncovered (duk_safe_call)
object [object Uint32Array] 128 7 512 4 object
false false false false false false false false false false true false false -> Uint32Array
false false false false false false false false false false true false false -> Uint32Array.prototype
final top: 0
==> rc=0, result='undefined'
===*/

/* It's not an error for the underlying plain buffer to be too small to
 * cover the slice.  This is allowed because it may happen for dynamic
 * and external buffers at run time anyway.  In any case, no memory
 * unsafe behavior happens.
 */
static duk_ret_t test_uncovered(duk_context *ctx) {
	duk_push_fixed_buffer(ctx, 256);
	duk_push_buffer_object(ctx, -1, 7, 512, DUK_BUFOBJ_UINT32ARRAY);

	duk_eval_string(ctx, "dumpBufferInfo");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_eval_string(ctx,
		"(function (v) {\n"
		"    for (var i = 0; i < v.length; i++) { v[i] = 123; }\n"
		"    for (var i = 0; i < v.length; i++) { var ignore = v[i]; }\n"
		"})");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_pop_n(ctx, 2);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_invalid_index1 (duk_safe_call)
==> rc=1, result='TypeError: buffer required, found none (stack index -2)'
*** test_invalid_index2 (duk_safe_call)
==> rc=1, result='TypeError: buffer required, found none (stack index -2147483648)'
===*/

static duk_ret_t test_invalid_index1(duk_context *ctx) {
	duk_push_fixed_buffer(ctx, 256);
	duk_push_buffer_object(ctx, -2, 7, 512, DUK_BUFOBJ_UINT32ARRAY);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_invalid_index2(duk_context *ctx) {
	duk_push_fixed_buffer(ctx, 256);
	duk_push_buffer_object(ctx, DUK_INVALID_INDEX, 7, 512, DUK_BUFOBJ_UINT32ARRAY);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_invalid_bufferobject (duk_safe_call)
==> rc=1, result='TypeError: buffer required, found [object Uint16Array] (stack index -1)'
*** test_invalid_string (duk_safe_call)
==> rc=1, result='TypeError: buffer required, found 'foobar' (stack index -1)'
*** test_invalid_null (duk_safe_call)
==> rc=1, result='TypeError: buffer required, found null (stack index -1)'
===*/

/* A bufferobject is -not- accepted as the underlying buffer.  This also
 * prevents issues like a slice being applied to a sliced view.
 */
static duk_ret_t test_invalid_bufferobject(duk_context *ctx) {
	duk_eval_string(ctx, "new Uint16Array(123);");
	duk_push_buffer_object(ctx, -1, 7, 512, DUK_BUFOBJ_UINT32ARRAY);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* A string is not allowed (although this might be useful). */
static duk_ret_t test_invalid_string(duk_context *ctx) {
	duk_push_string(ctx, "foobar");
	duk_push_buffer_object(ctx, -1, 7, 512, DUK_BUFOBJ_UINT32ARRAY);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_invalid_null(duk_context *ctx) {
	duk_push_null(ctx);
	duk_push_buffer_object(ctx, -1, 7, 512, DUK_BUFOBJ_UINT32ARRAY);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_invalid_flags1 (duk_safe_call)
final top: 2
==> rc=0, result='undefined'
*** test_invalid_flags2 (duk_safe_call)
==> rc=1, result='TypeError: invalid call args'
*** test_invalid_flags3 (duk_safe_call)
==> rc=1, result='TypeError: invalid call args'
===*/

/* If 'flags' is given as zero, it will match a DUK_BUFOBJ_DUKTAPEBUFFER.
 * So this test succeeds which is intentional.
 */
static duk_ret_t test_invalid_flags1(duk_context *ctx) {
	duk_push_fixed_buffer(ctx, 256);
	duk_push_buffer_object(ctx, -1, 7, 512, 0 /* no type given, but matches DUK_BUFOBJ_DUKTAPEBUFFER */);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

static duk_ret_t test_invalid_flags2(duk_context *ctx) {
	duk_push_fixed_buffer(ctx, 256);
	duk_push_buffer_object(ctx, -1, 7, 512, (duk_uint_t) 0xdeadbeef /* ERROR: bogus type */);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Boundary case. */
static duk_ret_t test_invalid_flags3(duk_context *ctx) {
	duk_push_fixed_buffer(ctx, 256);
	duk_push_buffer_object(ctx, -1, 7, 512, (duk_uint_t) (DUK_BUFOBJ_FLOAT64ARRAY + 1) /* ERROR: bogus type, right after last defined */);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_invalid_offlen_wrap1 (duk_safe_call)
==> rc=1, result='RangeError: invalid call args'
*** test_invalid_offlen_wrap2 (duk_safe_call)
==> rc=1, result='RangeError: invalid call args'
===*/

/* Byte offset + byte length wrap. */
static duk_ret_t test_invalid_offlen_wrap1(duk_context *ctx) {
	duk_push_fixed_buffer(ctx, 256);
	duk_push_buffer_object(ctx,
	                       -1,
	                       (duk_size_t) (duk_uint_t) -100,
	                       1000,
                               DUK_BUFOBJ_UINT8ARRAY);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/* Byte offset + byte length wrap, just barely */
static duk_ret_t test_invalid_offlen_wrap2(duk_context *ctx) {
	duk_push_fixed_buffer(ctx, 256);
	duk_push_buffer_object(ctx,
	                       -1,
	                       (duk_size_t) (duk_uint_t) -100,
	                       100,
                               DUK_BUFOBJ_UINT8ARRAY);
	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*===
*** test_allowed_offlen_nowrap1 (duk_safe_call)
object [object Uint8Array] 99 4294967196 99 1 object
false false false false false true false false false false false false false -> Uint8Array
false false false false false true false false false false false false false -> Uint8Array.prototype
final top: 1
==> rc=0, result='undefined'
===*/

/* Byte offset + byte length are just within limits of duk_uint_t and don't
 * wrap.  This works and doesn't cause a ~4G allocation because the conceptual
 * size (~4G) is unrelated to the underlying buffer size (256 bytes here).
 */
static duk_ret_t test_allowed_offlen_nowrap1(duk_context *ctx) {
	duk_push_fixed_buffer(ctx, 256);
	duk_push_buffer_object(ctx,
	                       -1,
	                       (duk_size_t) (duk_uint_t) -100,
	                       99,
                               DUK_BUFOBJ_UINT8ARRAY);
	duk_eval_string(ctx, "dumpBufferInfo");
	duk_dup(ctx, -2);
	duk_call(ctx, 1);
	duk_pop(ctx);

	duk_pop(ctx);

	printf("final top: %ld\n", (long) duk_get_top(ctx));
	return 0;
}

/*
 *  Main
 */

void test(duk_context *ctx) {
	register_dump_buffer_info(ctx);

	TEST_SAFE_CALL(test_basic);
	TEST_SAFE_CALL(test_view_buffer_prop);
	TEST_SAFE_CALL(test_unaligned);
	TEST_SAFE_CALL(test_unaligned_uneven);
	TEST_SAFE_CALL(test_uncovered);
	TEST_SAFE_CALL(test_invalid_index1);
	TEST_SAFE_CALL(test_invalid_index2);
	TEST_SAFE_CALL(test_invalid_bufferobject);
	TEST_SAFE_CALL(test_invalid_string);
	TEST_SAFE_CALL(test_invalid_null);
	TEST_SAFE_CALL(test_invalid_flags1);
	TEST_SAFE_CALL(test_invalid_flags2);
	TEST_SAFE_CALL(test_invalid_flags3);
	TEST_SAFE_CALL(test_invalid_offlen_wrap1);
	TEST_SAFE_CALL(test_invalid_offlen_wrap2);
	TEST_SAFE_CALL(test_allowed_offlen_nowrap1);

	/* XXX: testing for duk_size_t wrapping a duk_uint_t is only possible
	 * if duk_size_t is a larger type.
	 */
}
