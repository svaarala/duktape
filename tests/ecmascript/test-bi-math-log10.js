/*
 *  Math.log10()
 */

/*@include util-number.js@*/

/*===
function true false true
1
NaN
NaN
-Infinity
-Infinity
0
Infinity
1
3
88
3010300
===*/

function test() {
    var pd = Object.getOwnPropertyDescriptor(Math, 'log10');
    print(typeof pd.value, pd.writable, pd.enumerable, pd.configurable);
    print(Math.log10.length);

    printExact(Math.log10(0 / 0));
    printExact(Math.log10(-1));
    printExact(Math.log10(0));
    printExact(Math.log10(-0));
    printExact(Math.log10(1));
    printExact(Math.log10(1 / 0));

    printExact(Math.log10(10));
    printExact(Math.log10(1000));
    printExact(Math.log10(1e+88));
    printRounded6(Math.log10(1024));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
