/*
 *  Neutering.
 */

/*---
{
    "custom": true,
    "skip": true
}
---*/

/* Currently there is no neutering support, so just sketching what could be
 * covered by tests.
 */

function neuter(b) {
    // XXX: need an API to do this
}

function neuteredTest() {
    var b;
    var v;

    // .byteLength is required to read as zero.

    b = new ArrayBuffer(8); neuter(b);
    print(b.byteLength);

    // Duktape specific .length and .byteOffset should also read as zero.

    b = new ArrayBuffer(8); neuter(b);
    print(b.length);
    print(b.byteOffset);

    // ArrayBufferView byteOffset and byteLength must read as zero if the
    // referenced ArrayBuffer (note: not the same as duk_hbufferobject
    // internal 'buf' reference!) has been neutered.

    b = new ArrayBuffer(8); v = new Uint8Array(b, 3); neuter(b);
    print(v.byteOffset);
    print(v.byteLength);

    b = new ArrayBuffer(8); v = new DataView(b, 3); neuter(b);
    print(v.byteOffset);
    print(v.byteLength);

    // XXX: behavior for DataView and TypedArray constructors?

    // XXX: behavior when used as method arguments (e.g. .set(), .subarray())?
}

try {
    neuteredTest();
} catch (e) {
    print(e.stack || e);
}
