/*
 *  The algorithm for map() in E5.1 Section 15.4.4.19 seems to require that
 *  trailing non-existent elements of an array argument are effectively
 *  ignored, and don't count towards the final length of the result.  There
 *  is no explicit write for 'length' and step 8.c.iii is only executed
 *  if the element exists.  So, for non-existent trailing elements, no
 *  [[DefineOwnProperty]] is called and the result length shouldn't get updated.
 *
 *  Duktape 0.11.0 had this behavior:
 *
 *      ((o) Duktape 0.11.99
 *      duk> a = [1,2,3,,]
 *      = 1,2,3,
 *      duk> a.length
 *      = 4
 *      duk> b = a.map(function (x) { return x*x })
 *      = 1,4,9
 *      duk> b.length
 *      = 3
 *
 *  Other engines don't agree, e.g. V8:
 *
 *      > a = [1,2,3,,]
 *      [ 1, 2, 3,  ]
 *      > a.length
 *      4
 *      > b = a.map(function (x) { return x*x })
 *      [ 1, 4, 9,  ]
 *      > b.length
 *      4
 *
 *  Rhino:
 *
 *      Rhino 1.7 release 4 2013 08 27
 *      js> a = [1,2,3,,]
 *      1,2,3,
 *      js> a.length
 *      4
 *      js> b = a.map(function (x) { return x*x })
 *      1,4,9,
 *      js> b.length
 *      4
 *
 *  Test262 also checks for V8/Rhino-like behavior.  The map function only gets
 *  called for values that exist.
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
6 1 2 nonexistent nonexistent 3 nonexistent
map fn: number 1
map fn: number 2
map fn: number 3
6 1 4 nonexistent nonexistent 9 nonexistent
10 nonexistent nonexistent nonexistent 10 nonexistent nonexistent nonexistent 20 nonexistent nonexistent
map fn: number 10
map fn: number 20
10 nonexistent nonexistent nonexistent 100 nonexistent nonexistent nonexistent 400 nonexistent nonexistent
===*/

function test() {
    function mapper(x) {
        print('map fn:', typeof x, x);
        return x*x;
    }
    function dump(v) {
        var tmp = [];
        for (var i = 0; i < v.length; i++) {
            if (i in v) { tmp.push(v[i]) } else { tmp.push('nonexistent'); }
        }
        print(v.length, tmp.join(' '));
    }

    var a = [ 1, 2, , , 3, ,  ];  // -> items: 1 2 na na 3 na, length 6
    dump(a);

    var b = a.map(mapper);
    dump(b);

    var c = { '3': 10, '7': 20, 'length': 10 };
    dump(c);

    var d = Array.prototype.map.call(c, mapper);
    dump(d);
}

try {
    test();
} catch (e) {
    print(e);
}
