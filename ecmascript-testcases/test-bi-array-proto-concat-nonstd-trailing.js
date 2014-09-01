/*
 *  The algorithm in E5.1 Section 15.4.4.4 seems to require that trailing
 *  non-existent elements of an array argument are effectively ignored, and
 *  don't count towards the final length of the result.  There is no explicit
 *  write for 'length' and step 5.b.iii.3.b is only executed if the element
 *  exists.  So, for non-existent trailing elements, no [[DefineOwnProperty]]
 *  is called and the result length shouldn't get updated.
 *
 *  Duktape 0.11.0 had this behavior:
 *
 *      ((o) Duktape 0.11.99
 *      duk> a = [1,2,3]; b = [4,5,,]
 *      = 4,5,
 *      duk> a.length
 *      = 3
 *      duk> b.length
 *      = 3
 *      duk> a.concat(b)
 *      = 1,2,3,4,5
 *      duk> a.concat(b).length
 *      = 5
 *
 *  Other engines don't agree, e.g. V8:
 *
 *      > a = [1,2,3]; b = [4,5,,]
 *      [ 4, 5,  ]
 *      > a.length
 *      3
 *      > b.length
 *      3
 *      > a.concat(b)
 *      [ 1,
 *        2,
 *        3,
 *        4,
 *        5,
 *         ]
 *      > a.concat(b).length
 *      6
 *
 *  Rhino:
 *
 *      Rhino 1.7 release 4 2013 08 27
 *      js> a = [1,2,3]; b = [4,5,,]
 *      4,5,
 *      js> a.length
 *      3
 *      js> b.length
 *      3
 *      js> a.concat(b)
 *      1,2,3,4,5,
 *      js> a.concat(b).length
 *      6
 *
 *  Spidermonkey also agrees with V8 and Rhino.
 *
 *  The default behavior for Duktape will be changed to match the other engines,
 *  even though this seems non-standard.
 */

/*---
{
    "nonstandard": true
}
---*/

/*===
1,2,3
3
4,5,
3
6,7,
3
1,2,3,4,5,,6,7,
9
true true true true true false true true false
===*/

function test() {
    var a = [ 1, 2, 3 ];
    var b = [ 4, 5, , ];  // intermediate trailing elements are not ignored
    var c = [ 6, 7, , ];  // trailing ones are ignored by standard behavior
    var d = a.concat(b, c);
    var i;
    var tmp;

    print(a);
    print(a.length);
    print(b);
    print(b.length);
    print(c);
    print(c.length);
    print(d);
    print(d.length);

    tmp = [];
    for (i = 0; i < d.length; i++) {
        tmp.push(i in d);
    }
    print(tmp.join(' '));
}

try {
    test();
} catch (e) {
    print(e);
}
