/*
 *  Conditional operator (E5 Section 11.12).
 */

/*===
alt2
0 undefined 234
alt2
1 object 234
alt1
2 boolean 123
alt2
3 boolean 234
alt2
4 number 234
alt2
5 number 234
alt2
6 number 234
alt1
7 number 123
alt1
8 number 123
alt1
9 number 123
alt1
10 number 123
alt2
11 string 234
alt1
12 string 123
alt1
13 string 123
alt1
14 object 123
alt1
15 object 123
alt1
16 function 123
===*/

function test() {
    // Condition is evaluated with ToBoolean()
    var cond = [
        // undefined and null are considered false
        undefined, null,

        // booleans are as is
        true, false,

        // -0, +0, NaN are false; other numbers true
        -0, +0, 0/0,
        -1/0, -1e9, 1e9, 1/0,

        // empty string is false; other strings true
        '', '\0', 'foo',

        // all objects are true
        {}, [], function () {}
    ];

    cond.forEach(function (condValue, i) {
        try {
            // alternatives are evaluated only after checking the condition
            var t = condValue ? (print('alt1'), 123) : (print('alt2'), 234);
            print(i, typeof condValue, t);
        } catch (e) {
            print(i, typeof condValue, e);
        }
    });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
