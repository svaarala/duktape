/*
 *  Test for Symbol encoding behavior; for now encoded as an empty object.
 */

/*---
custom: true
---*/

/*===
a0
===*/

print(Duktape.enc('hex', CBOR.encode(Symbol('foo'))));
