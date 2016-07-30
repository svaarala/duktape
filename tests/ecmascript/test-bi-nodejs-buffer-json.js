/*
 *  Node.js Buffer JSON/JX/JC serialization
 */

/*---
{
    "custom": true
}
---*/

/*===
{"type":"Buffer","data":[66,67,254]}
{type:"Buffer",data:[66,67,254]}
{"type":"Buffer","data":[66,67,254]}
{"0":66,"1":67,"2":238}
|4243ee|
{"_buf":"4243ee"}
===*/

function nodejsBufferJsonTest() {
    // Node.js buffers go through toJSON() which supports slices.

    var b = new Buffer('ABCDEFGH');
    b = b.slice(1, 4);
    b[2] = 0xfe;

    print(JSON.stringify(b));
    print(Duktape.enc('jx', b));
    print(Duktape.enc('jc', b));

    // Also Node.js buffers are serialized natively (with slice support)
    // if the toJSON() method is not invoked.  We can make sure of that
    // by changing the Buffer prototype forcibly.

    var b = new Buffer('ABCDEFGH');
    b = b.slice(1, 4);
    b[2] = 0xee;
    Object.setPrototypeOf(b, Object.prototype);

    print(JSON.stringify(b));
    print(Duktape.enc('jx', b));
    print(Duktape.enc('jc', b));
}

try {
    nodejsBufferJsonTest();
} catch (e) {
    print(e.stack || e);
}
