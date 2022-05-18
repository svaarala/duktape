/*
 *  In Duktape 1.x Object.freeze() was allowed for a buffer object.  This is
 *  technically incorrect because the virtual array indices are not made
 *  non-writable (there's no internal support for doing so).  Duktape 2.x
 *  throws a TypeError in this situation, which matches V8 behavior.
 *
 *  ArrayBuffer has no virtual index properties so freeze is accepted
 *  in Duktape 3.x.  Also buffers with zero length are seal/freeze
 *  compatible.
 */

/*@include util-buffer.js@*/

/*===
0 [object Uint8Array] TypeError
1 [object ArrayBuffer] OK
2 [object Uint8Array] TypeError
3 [object Uint8Array] OK
4 [object Float32Array] TypeError
5 [object Uint8Array] TypeError
===*/

function test() {
    // Spot check some types.
    [
        createPlainBuffer(4),
        new ArrayBuffer(4),
        new Uint8Array(16),
        new Uint8Array(0),
        new Float32Array(16),
        new Buffer(16)
     ].forEach(function (v, i) {
        try {
            Object.freeze(v);
            print(i, Object.prototype.toString.call(v), 'OK');
        } catch (e) {
            print(i, Object.prototype.toString.call(v), e.name);
        }
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
