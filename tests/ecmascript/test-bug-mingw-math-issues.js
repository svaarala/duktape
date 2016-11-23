/*
 *  Some MinGW math issues
 *
 *  https://github.com/svaarala/duktape/pull/1099
 */

/*@include util-number.js@*/

/*===
1
1
-1e+100
-1e+100
NaN
NaN
NaN
NaN
0
-0
0
-0
0
-0
0
-0
NaN
NaN
NaN
NaN
NaN
NaN
0.7853981633974483
2.356194490192345
-0.7853981633974483
-2.356194490192345
1
1
===*/

function test() {
    printExact((0/0) ** 0);
    printExact((0/0) ** -0);
    printExact((-1e100) % (1/0));  // return 1st arg if modulus is +/- inf
    printExact((-1e100) % (-1/0));
    printExact((1/0) % (1/0));     // but if 1st arg is +/- inf (and modulus +/- inf), return NaN
    printExact((1/0) % (-1/0));
    printExact((-1/0) % (1/0));
    printExact((-1/0) % (-1/0));
    printExact(0 % 1);  // preserve zero and its sign, even for inf
    printExact(-0 % 1);
    printExact(0 % -1);
    printExact(-0 % -1);
    printExact(0 % (1/0));
    printExact(-0 % (1/0));
    printExact(0 % (-1/0));
    printExact(-0 % (-1/0));
    printExact(0 % (0/0));    // ... but if modulus is NaN or 0, result is NaN
    printExact(-0 % (0/0));
    printExact(0 % 0);
    printExact(-0 % 0);
    printExact(0 % -0);
    printExact(-0 % -0);
    printExact(Math.atan2(1/0, 1/0));
    printExact(Math.atan2(1/0, -1/0));
    printExact(Math.atan2(-1/0, 1/0));
    printExact(Math.atan2(-1/0, -1/0));
    printExact(Math.pow(0/0, 0));
    printExact(Math.pow(0/0, -0));
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
