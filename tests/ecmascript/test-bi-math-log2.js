/*
 *  Math.log2()
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
10
812
9965784
===*/

function test() {
    var pd = Object.getOwnPropertyDescriptor(Math, 'log2');
    print(typeof pd.value, pd.writable, pd.enumerable, pd.configurable);
    print(Math.log2.length);

    printExact(Math.log2(NaN));
    printExact(Math.log2(-1));
    printExact(Math.log2(0));
    printExact(Math.log2(-0));
    printExact(Math.log2(1));
    printExact(Math.log2(Infinity));

    printExact(Math.log2(2));
    printExact(Math.log2(1024));
    printExact(Math.log2(2.73121871170759e+244));
    printRounded6(Math.log2(1000));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
