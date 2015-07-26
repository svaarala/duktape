/*===
1e+2
8e+2
1e+3
1.23e+4
9.88e+4
1.00e+5
1.2345e+4
1e-1
0e+0
0e+0
0.0e+0
0.00e+0
0e+0
0e+0
0.0e+0
0.00e+0
===*/

function expTest() {
    function test(x,n) {
        print(new Number(x).toExponential(n));
    }

    test(123, 0);
    test(750, 0);  // rounds up
    test(999, 0);  // rounds up, over first digit

    test(12345, 2);
    test(98750, 2);  // rounds up
    test(99999, 2);  // rounds up, over first digit

    test(12345, undefined);  // minimum length (Note: Rhino outputs "1e+4")
    test(0.1, undefined);  // minimum length

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
    expTest();
} catch (e) {
    print(e);
}
