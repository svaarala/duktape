/*
 *  Test for Symbol encoding behavior; for now encoded as an empty object.
 */

/*---
{
    "skip": true
}
---*/

/*===
a0
===*/

try {
    print(Duktape.enc('hex', CBOR.encode(Symbol('foo'))));
} catch (e) {
    print(e.stack || e);
}
