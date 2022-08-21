/*
 *  Test case conversions in case insensitive matching.  See
 *  comments in E5 Section 15.10.2.8.
 */

var r, t;

/*===
foo
Foo
fOO
===*/

r = /fOO/i;
t = r.exec('foo'); print(t);
t = r.exec('Foo'); print(t);
t = r.exec('fOO'); print(t);

/* XXX: add more cases */
