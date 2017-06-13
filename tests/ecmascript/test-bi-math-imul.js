/*
 *  Math.imul()
 */

/*@include util-number.js@*/

/*===
function true false true
2
--- 0
0
0
0
0
0
0
0
0
0
valueOf 1
0
valueOf 2
toString 2
0
--- 1
0
1253409321
-1088058497
0
0
1088058497
1231910301
-1253409321
0
valueOf 1
1088058497
valueOf 2
toString 2
-744461299
--- 2
0
-1088058497
15129
0
0
-15129
42287915
1088058497
0
valueOf 1
-15129
valueOf 2
toString 2
39483
--- 3
0
0
0
0
0
0
0
0
0
valueOf 1
0
valueOf 2
toString 2
0
--- 4
0
0
0
0
0
0
0
0
0
valueOf 1
0
valueOf 2
toString 2
0
--- 5
0
1088058497
-15129
0
0
15129
-42287915
-1088058497
0
valueOf 1
15129
valueOf 2
toString 2
-39483
--- 6
0
1231910301
42287915
0
0
-42287915
560833313
-1231910301
0
valueOf 1
-42287915
valueOf 2
toString 2
-937191855
--- 7
0
-1253409321
1088058497
0
0
-1088058497
-1231910301
1253409321
0
valueOf 1
-1088058497
valueOf 2
toString 2
744461299
--- 8
0
0
0
0
0
0
0
0
0
valueOf 1
0
valueOf 2
toString 2
0
--- 9
valueOf 1
0
valueOf 1
1088058497
valueOf 1
-15129
valueOf 1
0
valueOf 1
0
valueOf 1
15129
valueOf 1
-42287915
valueOf 1
-1088058497
valueOf 1
0
valueOf 1
valueOf 1
15129
valueOf 1
valueOf 2
toString 2
-39483
--- 10
valueOf 2
toString 2
0
valueOf 2
toString 2
-744461299
valueOf 2
toString 2
39483
valueOf 2
toString 2
0
valueOf 2
toString 2
0
valueOf 2
toString 2
-39483
valueOf 2
toString 2
-937191855
valueOf 2
toString 2
744461299
valueOf 2
toString 2
0
valueOf 2
toString 2
valueOf 1
-39483
valueOf 2
toString 2
valueOf 2
toString 2
103041
- coercion order
valueOf 1
valueOf 2
-28782
- argument counts
0
0
6
-1088058497
===*/

function test() {
    var pd = Object.getOwnPropertyDescriptor(Math, 'imul');
    print(typeof pd.value, pd.writable, pd.enumerable, pd.configurable);
    print(Math.imul.length);

    var values = [
        -1/0, -0x123456789abcd, -123.1, -0, 0,
        123.1, 0xdeadbeef, 0x123456789abcd, 1/0,
      { valueOf: function () { print('valueOf 1'); return 123 },
        toString: function () { print('toString 1'); return -321; } },
      { valueOf: function () { print('valueOf 2'); return {} },  // fails
        toString: function () { print('toString 2'); return -321; } },
    ];

    values.forEach(function (v1, i1) {
        print('---', i1);
        values.forEach(function (v2, i2) {
            printExact(Math.imul(v1, v2));
        });
    });

    // Coercion order: left-to-right.
    print('- coercion order');
    printExact(Math.imul({ valueOf: function() { print('valueOf 1'); return 123; } },
                         { valueOf: function() { print('valueOf 2'); return -234; } }));

    // Two operands only; if one given, other operand coerces to 0, result 0.
    // If more than two given, rest are ignored.
    print('- argument counts');
    printExact(Math.imul());
    printExact(Math.imul(0x123456789abcd));
    printExact(Math.imul(2, 3, 4));
    printExact(Math.imul(0x123456789abcd, 123, -321));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
