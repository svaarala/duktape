/*
 *  When Duktape.Buffer() is called with a duk_hbufferobject argument,
 *  the returned plain buffer is the underlying buffer ('buf') of the
 *  duk_hbufferobject.  Any slice/view information is lost because it
 *  cannot be represented by a plain buffer value.
 *
 *  Similarly, new Duktape.Buffer() will extract the underlying 'buf'
 *  object and create a Duktape.Buffer object matching the entire
 *  underlying buffer, ignoring slice/view information.
 *
 *  This behavior is compatible with Duktape 1.2 behavior and perhaps
 *  the most logical one assuming that the return value is a plain buffer
 *  (and the plain buffer type is not extended with slice support).
 *
 *  Overall, there are three logical behaviors:
 *
 *    1. Return the underlying 'buf' value regardless of any slice/view
 *       information.  This is compatible with previous (Duktape 1.2)
 *       semantics; for Duktape 1.2 the argument is a Duktape.Buffer which
 *       was always 1:1 mapped, but is a bit confusing for slices/views.
 *
 *    2. Return a plain buffer value which is a copy of the input slice.
 *       This is intuitive but creates an (often) unnecessary copy, and
 *       is also a change to Duktape 1.2 behavior.
 *
 *    3. Return the buffer as-is for 1:1 mapped buffers and a copy for
 *       proper slices.  Compatible with Duktape 1.2 behavior but may be
 *       misleading.
 *
 *  As described above, current behavior is alternative 1.
 */

/*@include util-typedarray.js@*/

/*===
emulated slice copy test
5 bytes: 4344454647
5 bytes: 5844454647
8 bytes: 4142584445464748
8
ABXDEFGH
5 bytes: 5844454647
5 bytes: 5844454647
5 bytes: 5a44454647
object object false
===*/

function emulatedSliceCopyTest() {
    /*
     *  Example of how a caller can coerce a proper slice/view into a plain
     *  buffer containing only the slice data (not the whole underlying
     *  buffer).
     */

    var buf, buf2;
    var view, view2;
    var tmp;
    var plainbuf;
    var i;

    // 'buf' is "ABCDEFGH" and 'view' maps to "CDEFG" of it.
    buf = new ArrayBuffer(8);
    tmp = new Uint8Array(buf);
    for (i = 0; i < tmp.length; i++) {
        tmp[i] = 0x41 + i;
    }
    view = tmp.subarray(2, 7);
    printTypedArray(view);
    view[0] = ('X').charCodeAt(0);
    printTypedArray(view);          // -> XDEFG
    printTypedArray(buf);           // -> ABXDEFGH (shared underlying buffer)

    // 'plainbuf' will be the underlying, plain buffer of 'view' without the
    // slice offsets, i.e. the 8-byte buffer containing 'ABXDEFGH'.
    plainbuf = Duktape.Buffer(view);
    print(plainbuf.length);  // --> 8
    print(String(plainbuf)); // --> ABXDEFGH

    // Create a new ArrayBuffer and copy the 'view' slice into it.
    buf2 = new ArrayBuffer(view.byteLength);
    (new Uint8Array(buf2)).set(new Uint8Array(view.buffer, view.byteOffset, view.byteLength));
    printTypedArray(buf2);         // -> XDEFG

    // Demonstrate independence.

    view2 = new Uint8Array(buf2);
    view2[0] = ('Z').charCodeAt(0);
    printTypedArray(view);           // -> XDEFG
    printTypedArray(view2);          // -> ZDEFG
    print(typeof view.buffer, typeof view2.buffer, view.buffer === view2.buffer);
}

try {
    print('emulated slice copy test');
    emulatedSliceCopyTest();
} catch (e) {
    print(e.stack || e);
}
