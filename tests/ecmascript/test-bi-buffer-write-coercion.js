/*
 *  Plain buffer and Duktape.Buffer() write coercion was changed in
 *  Duktape 1.3.0 from:
 *
 *      (duk_uint8_t) duk_to_int()
 *
 *  to:
 *
 *      (duk_uint8_t) duk_to_uint32()
 *
 *  The change was made for consistency with Node.js Buffer and TypedArray
 *  bindings.  (Duktape 1.1.x had a different coercion still.)
 *
 *  The differences are very slight (e.g. -1 still maps to 0xff), and
 *  mostly come into play when duk_to_int() would have clamped a value
 *  to DUK_INT_MIN or DUK_INT_MAX.
 *
 *  Because DUK_INT_MIN is 0x80000000 (or 0x80000000'00000000), any
 *  clamped value would be truncated to zero.  For this case there's
 *  no difference to duk_to_uint32() coercion, e.g. -Infinity coerces
 *  to 0x00 in both.
 *
 *  But DUK_INT_MAX is 0x7fffffff (or 0x7fffffff'ffffffff) which clamps
 *  to 0xff.  This differs from duk_to_uint32() coercion, where e.g.
 *  +Infinity coerces to 0x00 (rather than 0xff).
 */

/*===
0
0
255
255
0
0
0
0
===*/

function bufferWriteCoercionTest() {
    var b = Duktape.Buffer(1);
    var B = new Duktape.Buffer(1);

    b[0] = -1 / 0;
    B[0] = -1 / 0;
    print(b[0]);  // 0x00 in both Duktape 1.2 and Duktape 1.3
    print(B[0]);  // same

    b[0] = -1;
    B[0] = -1;
    print(b[0]);  // 0xFF in both Duktape 1.2 and Duktape 1.3
    print(B[0]);  // same

    b[0] = 1 / 0;
    B[0] = 1 / 0;
    print(b[0]);  // 0xFF in Duktape 1.2, 0x00 in Duktape 1.3
    print(B[0]);  // same

    b[0] = 0 / 0;
    B[0] = 0 / 0;
    print(b[0]);  // 0x00 in both Duktape 1.2 and Duktape 1.3
    print(B[0]);  // same
}

try {
    bufferWriteCoercionTest();
} catch (e) {
    print(e.stack || e);
}
