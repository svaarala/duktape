/*===
1.000
900000000000000000000.00000000000000000000
1e+21
0
1
0
1
0
0
0.0
0.00
0
0
0.0
0.00
===*/

function fixedTest() {
    function test(x, n) {
        print(new Number(x).toFixed(n));
    }

    test(1, 3);
    test(9e20, 20);
    test(1e21, 20);  // falls back to ToString()

    test(0.1, 0);
    test(0.9, 0);  // rounds up
    test(0.1, undefined);
    test(0.9, undefined);  // rounds up

    test(+0, undefined);
    test(+0, 0);
    test(+0, 1);
    test(+0, 2);
    test(-0, undefined);
    test(-0, 0);
    test(-0, 1);
    test(-0, 2);
}

try {
    fixedTest();
} catch (e) {
    print(e);
}

