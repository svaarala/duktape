/*@include util-buffer.js@*/

/*===
Node.js Buffer methods
- compare
0
0
- isBuffer
false
true
- byteLength
19
16
- concat
{type:"Buffer",data:[255,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,253,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112]}
===*/

function nodejsBufferMethodTest() {
    var pb, nb, t;

    pb = createPlainBuffer('abcdefghijklmnop');
    nb = new Buffer('abcdefghijklmnop');

    /* While Node.js Buffer methods are intended for Buffer instances only,
     * they also work for ArrayBuffers and plain buffers in Duktape.
     */

    print('- compare');
    print(Buffer.compare(pb, pb));
    print(Buffer.compare(pb, nb));

    print('- isBuffer');
    print(Buffer.isBuffer(pb));
    print(Buffer.isBuffer(nb));

    // XXX: for now follows old Node.js behavior where Buffer.byteLength()
    // would string coerce its argument; plain buffer (usually) coerces to
    // '[object Uint8Array]' whose length is 19, Node.js Buffer coerces
    // (currently) to a string with the same bytes as the buffer so it comes
    // out more usefully as 16 here.
    print('- byteLength');
    print(Buffer.byteLength(pb));
    print(Buffer.byteLength(nb));

    print('- concat');
    pb[0] = 255;
    nb[0] = 253;
    t = Buffer.concat([ pb, nb ]);
    print(Duktape.enc('jx', t));
}

try {
    print('Node.js Buffer methods');
    nodejsBufferMethodTest();
} catch (e) {
    print(e.stack || e);
}
