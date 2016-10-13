/*
 *  Calling .subarray() on a Node.js Buffer instance invokes
 *  Uint8Array.prototype.subarray() which creates a plain Uint8Array
 *  rather than another Node.js Buffer.  The difference is just a matter
 *  of inheritance though - both are Uint8Arrays as far as internal typing
 *  is concerned.
 */

/*===
true
true
true
[object Uint8Array]
false
false
true
[object Uint8Array]
===*/

function test() {
    var buf = new Buffer('abcdefgh');
    print(Buffer.isBuffer(buf));
    print(buf instanceof Buffer);
    print(buf instanceof Uint8Array);
    print(Object.prototype.toString.call(buf));

    var sub = buf.subarray(1);
    print(Buffer.isBuffer(sub));
    print(sub instanceof Buffer);
    print(sub instanceof Uint8Array);
    print(Object.prototype.toString.call(sub));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
