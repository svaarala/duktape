/*@include util-buffer.js@*/

/*===
TypedArray.prototype methods
- set
|6162636465666768696a6b6c6d6e6f70|
|6162636465660102036a6b6c6d6e6f70|
|6162636465660102036a6b6cfffefd70|
|61fefd6465660102036a6b6cfffefd70|
- subarray
object [object Uint8Array] false
|65666768696a6b6c6d|
4 9 1
|65ff6768696a6b6c6d|
object [object Uint8Array] false
|6162636465ff6768696a6b6c6d6e6f70|
0 16 1
|6162636465ff6768696a6b6c6d6e6f70|
===*/

function typedArrayPrototypeMethodTest() {
    var pb, t;

    pb = createPlainBuffer('abcdefghijklmnop');

    // typedarray.prototype.set
    print('- set');
    print(Duktape.enc('jx', pb));
    Uint16Array.prototype.set.call(pb, [ 1, 2, 3 ], 6);  // as 'this'
    print(Duktape.enc('jx', pb));
    t = createPlainBuffer(3);
    t[0] = 0xff;
    t[1] = 0xfe;
    t[2] = 0xfd;
    Uint16Array.prototype.set.call(pb, t, 12);  // as 'this' and source
    print(Duktape.enc('jx', pb));
    Uint16Array.prototype.set.call(pb, t.subarray(1), 1);  // as 'this' and source, sliced
    print(Duktape.enc('jx', pb));

    // typedarray.prototype.subarray() creates a new view into an existing
    // backing buffer.  When the typedarray method is forcibly called with
    // a plain buffer as the 'this' binding (which is non-standard behavior
    // anyway) we can't return a plain buffer result because slice information
    // cannot be recorded by a plain buffer.  Instead, the call behaves as if
    // the input buffer was coerced into a full Uint8Array object.

    pb = createPlainBuffer('abcdefghijklmnop');
    print('- subarray');

    t = Uint8Array.prototype.subarray.call(pb, 4, -3);
    print(typeof t, t, isPlainBuffer(t));
    print(Duktape.enc('jx', t));
    print(t.byteOffset, t.byteLength, t.BYTES_PER_ELEMENT);
    pb[5] = 0xff;  // demonstrate pb and t share same storage
    print(Duktape.enc('jx', t));

    t = Uint8Array.prototype.subarray.call(pb);  // full index span, i.e. no actual view offset/limit
    print(typeof t, t, isPlainBuffer(t));
    print(Duktape.enc('jx', t));
    print(t.byteOffset, t.byteLength, t.BYTES_PER_ELEMENT);
    pb[5] = 0xff;  // demonstrate pb and t share same storage
    print(Duktape.enc('jx', t));
}

try {
    print('TypedArray.prototype methods');
    typedArrayPrototypeMethodTest();
} catch (e) {
    print(e.stack || e);
}
