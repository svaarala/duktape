/*
 *  Math.hypot()
 */

/*@include util-number.js@*/

/*===
0
NaN
NaN
NaN
Infinity
Infinity
NaN
Infinity
NaN
NaN
NaN
0
5
5
7.0710678118654755
7.0710678118654755
812
0
0
0
0
0
0
arg1
arg2
arg3
arg4
Infinity
===*/

printExact(Math.hypot());
printExact(Math.hypot(NaN, 1));
printExact(Math.hypot(1, NaN));
printExact(Math.hypot(NaN, NaN));
printExact(Math.hypot(Infinity, NaN));
printExact(Math.hypot(NaN, -Infinity));
printExact(Math.hypot(1, 2, NaN));
printExact(Math.hypot(1, 2, NaN, Infinity));
printExact(Math.hypot(1, 2, 'pig'));
printExact(Math.hypot('pig', 'cow'));
printExact(Math.hypot('pig', 'cow', 'ape'));
printExact(Math.hypot(-0));

printExact(Math.hypot(3, 4));
printExact(Math.hypot('3', '4'));
printExact(Math.hypot(3, 4, 5));
printExact(Math.hypot('3', '4', '5'));
printExact(Math.hypot(-812));

printExact(Math.hypot(-0));
printExact(Math.hypot(+0));
printExact(Math.hypot(-0, -0));
printExact(Math.hypot(-0, +0));
printExact(Math.hypot(+0, -0));
printExact(Math.hypot(+0, +0));

printExact(Math.hypot({ valueOf: function () { print('arg1'); return 123.0; } },
               { valueOf: function () { print('arg2'); return 0/0; } },
               { valueOf: function () { print('arg3'); return 1/0; } },
               { valueOf: function () { print('arg4'); return -123.0; } }));


/*===
11814043422.538206
===*/
print(Math.hypot(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 1e5, -10004,
                 2, 4.5, 6.7, 8.9, 0xdeadbeef,
                 2, 4.5, 6.7, 8.9, 0xdeadbeef,
                 2, 4.5, 6.7, 8.9, 0xdeadbeef,
                 2, 4.5, 6.7, 8.9, 0xdeadbeef,
                 2, 4.5, 6.7, 8.9, 0xdeadbeef,
                 2, 4.5, 6.7, 8.9, 0xdeadbeef,
                 2, 4.5, 6.7, 8.9, 0xdeadbeef,
                 2, 4.5, 6.7, 8.9, 0xdeadbeef,
                 2, 4.5, 6.7, 8.9, 0xdeadbeef,
                 2, 4.5, 6.7, 8.9, 0xdeadbeef));

/*===
function true false true
2
===*/

function test() {
    var pd = Object.getOwnPropertyDescriptor(Math, 'hypot');
    print(typeof pd.value, pd.writable, pd.enumerable, pd.configurable);
    print(Math.hypot.length);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
