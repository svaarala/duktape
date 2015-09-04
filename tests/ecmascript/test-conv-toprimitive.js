/*
 *  ToPrimitive() (E5 Section 9.1).
 */

/*===
ToPrimitive() with hint "number"
0 number NaN
1 number 0
2 number 1
3 number 0
4 number 123
5 number NaN
6 number NaN
7 TypeError
valueOf 0 object
8 number 123
toString 0 object
9 number 234
valueOf 0 object
10 number 1
toString 0 object
11 number 345
valueOf 0 object
12 number NaN
toString 0 object
13 TypeError
valueOf
14 number 123
valueOf
toString
15 number 234
valueOf
16 number 123
toString
17 number 1
valueOf
18 number 0
valueOf
19 Error
valueOf
20 number 123
valueOf
21 Error
22 TypeError
valueOf called
toString called
23 TypeError
ToPrimitive() with hint "string"
0 string undefined
1 string null
2 string true
3 string false
4 string 123
5 string foo
6 string [object Object]
7 TypeError
8 string [object Object]
toString 0 object
9 string 234
10 string [object Object]
toString 0 object
11 string 345
12 string [object Object]
toString 0 object
13 TypeError
toString
14 string 234
toString
15 string 234
toString
valueOf
16 string 123
toString
17 string true
valueOf
18 string false
toString
19 string 234
toString
20 Error
toString
21 Error
22 TypeError
toString called
valueOf called
23 TypeError
ToPrimitive() with no hint
0 string undefined
1 string null
2 string true
3 string false
4 string 123
5 string foo
6 string [object Object]
7 TypeError
valueOf 0 object
8 string 123
toString 0 object
9 string 234
valueOf 0 object
10 string true
toString 0 object
11 string 345
valueOf 0 object
12 string [object Object]
toString 0 object
13 TypeError
valueOf
14 string 123
valueOf
toString
15 string 234
valueOf
16 string 123
toString
17 string true
valueOf
18 string false
valueOf
19 Error
valueOf
20 string 123
valueOf
21 Error
22 TypeError
valueOf called
toString called
23 TypeError
===*/

function test() {
    var obj = {};

    Object.setPrototypeOf(obj, null);  // object with no prototype

    // NOTE: in tests where valueOf/toString is nominally missing,
    // a function is inherited from Object.prototype unless the object
    // is explicitly created without a prototype.

    var testValues = [
        undefined, null, true, false, 123.0, 'foo',

        // object with neither valueOf nor toString, but inherited
        {},

        // object with neither valueOf nor toString, and not inherited
        obj,

        // object with valueOf only
        { valueOf: function () { print('valueOf', arguments.length, typeof this); return 123; } },

        // object with toString only
        { toString: function () { print('toString', arguments.length, typeof this); return '234'; } },

        // object with valueOf only, valueOf returns primitive but not number
        { valueOf: function () { print('valueOf', arguments.length, typeof this); return true; } },

        // object with toString only, toString returns primitive but not string
        { toString: function () { print('toString', arguments.length, typeof this); return 345; } },

        // object with valueOf only, valueOf returns non-primitive
        { valueOf: function () { print('valueOf', arguments.length, typeof this); return {}; } },

        // object with toString only, toString returns non-primitive
        { toString: function () { print('toString', arguments.length, typeof this); return {}; } },

        // object with both valueOf and toString, both return primitive
        { valueOf: function () { print('valueOf'); return 123; },
          toString: function () { print('toString'); return '234'; } },

        // object with both valueOf and toString, valueOf returns non-primitive
        { valueOf: function () { print('valueOf'); return {}; },
          toString: function () { print('toString'); return '234'; } },

        // object with both valueOf and toString, toString returns non-primitive
        { valueOf: function () { print('valueOf'); return 123; },
          toString: function () { print('toString'); return {}; } },

        // object with valueOf non-callable, toString returns a
        // primitive (but non-string which is accepted)
        { valueOf: 123,
          toString: function () { print('toString'); return true; } },

        // object with toString non-callable, valueOf returns a
        // primitive
        { valueOf: function () { print('valueOf'); return false; },
          toString: 234 },

        // object with valueOf callable but throws error
        { valueOf: function () { print('valueOf'); throw new Error('valueOf throw'); },
          toString: function () { print('toString'); return '234'; } },

        // object with toString callable but throws error
        { valueOf: function () { print('valueOf'); return 123; },
          toString: function () { print('toString'); throw new Error('toString throw'); } },

        // object with both valueOf and toString callable but both throw an error
        { valueOf: function () { print('valueOf'); throw new Error('valueOf throw'); },
          toString: function () { print('toString'); throw new Error('toString throw'); } },

        // neither valueOf nor toString is callable
        { valueOf: 123, toString: 234 },

        // both valueOf and toString are callable but return non-primitive
        { valueOf: function () { print('valueOf called'); return {} },
          toString: function () { print('toString called'); return {} } },
    ];

    // ToPrimitive() with hint "number":
    // - valueOf is called if callable, otherwise skipped; if result is
    //   primitive return that, otherwise continue
    // - toString is called if callable, otherwise skipped; if result is
    //   pritimive return that, otherwise continue
    // - TypeError

    print('ToPrimitive() with hint "number"');
    testValues.forEach(function (v, i) {
        try {
            // ToNumber() calls ToPrimitive() with hint number when value is object,
            // and then coerces the primitive result using ToNumber() which always
            // succeeds.
            var t = +v;
            print(i, typeof t, t);
        } catch (e) {
            print(i, e.name);
        }
    });

    // ToPrimitive() with hint "string": same but with order reversed
    // for valueOf and toString calls.

    print('ToPrimitive() with hint "string"');
    testValues.forEach(function (v, i) {
        try {
            // ToString calls ToPrimitive() with hint string when value is object,
            // and then coerces the primitive result using ToString() which always
            // succeeds.
            var t = String(v);
            print(i, typeof t, t);
        } catch (e) {
            print(i, e.name);
        }
    });

    print('ToPrimitive() with no hint');
    testValues.forEach(function (v, i) {
        try {
            // Addition calls ToPrimitive() on each argument (with no prechecks)
            // and then coerces them both to string if either is a string.
            var t = "" + v;
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
