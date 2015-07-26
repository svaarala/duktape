/*
 *  The algorithm for concat() in E5.1 Section 15.4.4.4 seems to require that
 *  trailing non-existent elements of an array argument are effectively
 *  ignored, and don't count towards the final length of the result.  There
 *  is no explicit write for 'length' and step 5.b.iii.3.b is only executed
 *  if the element exists.  So, for non-existent trailing elements, no
 *  [[DefineOwnProperty]] is called and the result length shouldn't get updated.
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
3 1 2 3
3 4 5 nonexistent
3 6 7 nonexistent
9 1 2 3 4 5 nonexistent 6 7 nonexistent
2 [object Object] [object Object]
===*/

function test() {
    function dump(v) {
        var tmp = [];
        for (var i = 0; i < v.length; i++) {
            if (i in v) { tmp.push(v[i]) } else { tmp.push('nonexistent'); }
        }
        print(v.length, tmp.join(' '));
    }

    var a = [ 1, 2, 3 ];
    dump(a);

    var b = [ 4, 5, , ];  // intermediate trailing elements are not ignored
    dump(b);

    var c = [ 6, 7, , ];  // trailing ones are ignored by standard behavior
    dump(c);

    var d = a.concat(b, c);
    dump(d);

    // concat() doesn't unflatten non-Array objects, so this results in a
    // two-element array: [e, f]

    var e = { '3': 'foo', '7': 'bar', 'length': 10 };
    var f = { '0': 'quux', '2': 'baz', 'length': 5 };
    var g = Array.prototype.concat.call(e, f);
    dump(g);
}

try {
    test();
} catch (e) {
    print(e);
}
