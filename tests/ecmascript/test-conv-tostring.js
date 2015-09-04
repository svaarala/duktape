/*
 *  ToString() (E5 Section 9.8).
 *
 *  String(v) returns ToString() with no modifications or pre-steps.
 */

/* Custom because of function ToString() coercion */
/*---
{
    "custom": true
}
---*/

/*===
0 string undefined
1 string null
2 string true
3 string false
4 string 123
5 string foo
6 string function myfunc() {|* ecmascript *|}
7 string [object Object]
8 TypeError
9 string foo
10 string 123
11 string 123
12 string 234
===*/

function test() {
    var obj = {};
    Object.setPrototypeOf(obj, null);

    [ undefined, null, true, false, 123.0, 'foo', function myfunc() {},

      // For objects, coercion is ToString(ToPrimitive(obj, hint String))

      {},
      obj,
      { toString: function () { return 'foo'; } },
      { toString: function () { return 123; } },
      { toString: function () { return 123; },
        valueOf: function () { return 234; } },
      { toString: 'non-callable',
        valueOf: function () { return 234; } },

      // ToPrimitive() test covers a lot more variants.
    ].forEach(function (v, i) {
        try {
            var t = String(v);
            if (typeof v === 'function' && typeof t === 'string') {
                // expect string hack
                t = t.replace(/\//g, '|');
            }
            print(i, typeof t, t);
        } catch (e) {
            print(i, e.name);
        }
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
