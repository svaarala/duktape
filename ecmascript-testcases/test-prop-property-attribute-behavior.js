/*
 *  Test property attribute behavior in various special cases.
 *  Use Object built-in to read and write property descriptors,
 *  and check that their behavior also matches.
 */

/*---
{
    "skip": true
}
---*/

/* FIXME */

/*===
===*/

x = {};
Object.defineProperty(x, 'foo', {enumerable: true, configurable: true, value: "bar", writable: true });
