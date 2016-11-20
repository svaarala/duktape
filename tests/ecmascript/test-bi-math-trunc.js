/*
 *  Math.trunc()
 */

/*@include util-number.js@*/

/*===
function true false true
1
NaN
-0
0
Infinity
-Infinity
0
-0
10
9000
-1
-812
812
===*/

function test() {
    var pd = Object.getOwnPropertyDescriptor(Math, 'trunc');
    print(typeof pd.value, pd.writable, pd.enumerable, pd.configurable);
    print(Math.trunc.length);

    printExact(Math.trunc(0 / 0));
    printExact(Math.trunc(-0));
    printExact(Math.trunc(0));
    printExact(Math.trunc(1 / 0));
    printExact(Math.trunc(-1 / 0));
    printExact(Math.trunc(0.5));
    printExact(Math.trunc(-0.5));

    printExact(Math.trunc(10.5));
    printExact(Math.trunc(9000.5));
    printExact(Math.trunc(-1.5));
    printExact(Math.trunc(-812.88));
    printExact(Math.trunc(812.88));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
