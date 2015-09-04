/*
 *  Literal reference (E5 Section 11.1.3).
 */

/*===
0 undefined [object Undefined]
1 object [object Null]
2 boolean [object Boolean]
3 boolean [object Boolean]
4 number [object Number]
5 number [object Number]
6 number [object Number]
7 number [object Number]
8 number [object Number]
9 number [object Number]
10 number [object Number]
11 number [object Number]
12 number [object Number]
13 number [object Number]
14 number [object Number]
15 number [object Number]
16 string [object String]
17 string [object String]
18 string [object String]
19 object [object RegExp]
===*/

[
    undefined,   // not actually a literal, but global variable

    null,
    true, false,
    -1e9, -123456789, -1, -0, +0, 1, 123456789, 1e9,
    -0xdeadbeef, 0xdeadbeef,
    -123.456, 123.456,
    '', 'foo', 'foo\u0041',
    /foo/gim
].forEach(function (v, i) {
    print(i, typeof v, Object.prototype.toString.call(v));
});
