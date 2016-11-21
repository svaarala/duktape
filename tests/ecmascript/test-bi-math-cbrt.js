/*
 *  Math.cbrt()
 */

/*@include util-number.js@*/

/*===
function true false true
1
0
-0
Infinity
-Infinity
NaN
2
4
10
-2
2.5
-2.5
812
===*/

function test() {
    var pd = Object.getOwnPropertyDescriptor(Math, 'cbrt');
    print(typeof pd.value, pd.writable, pd.enumerable, pd.configurable);
    print(Math.cbrt.length);

    printExact(Math.cbrt(0));
    printExact(Math.cbrt(-0));
    printExact(Math.cbrt(1 / 0));
    printExact(Math.cbrt(-1 / 0));
    printExact(Math.cbrt(0 / 0));

    printExact(Math.cbrt(8));
    printExact(Math.cbrt(64));
    printExact(Math.cbrt(1000));
    printExact(Math.cbrt(-8));
    printExact(Math.cbrt(15.625));
    printExact(Math.cbrt(-15.625));
    printExact(Math.cbrt(535387328));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
