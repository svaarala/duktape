/*
 *  Test case conversions in case insensitive matching.  See
 *  comments in E5 Section 15.10.2.8.
 */

/*---
{
    "skip": true
}
---*/

/*FIXME*/

var r, t;

/*===
foo
Foo
fOO
===*/

try {
    r = /fOO/i;
    t = r.exec('foo'); print(t);
    t = r.exec('Foo'); print(t);
    t = r.exec('fOO'); print(t);
} catch (e) {
    print(e.name);
}
