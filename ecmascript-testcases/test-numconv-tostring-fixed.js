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
0.00000000100000000000
0.0000000010
0.000000001
0.00000000
0.0000000
0.00000000800000000000
0.0000000080
0.000000008
0.00000001
0.0000000
0.00000000900000000000
0.0000000090
0.000000009
0.00000001
0.0000000
0.99999999900000002828
0.9999999990
0.999999999
1.00000000
1.0000000
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

    test(1e-9, 20);
    test(1e-9, 10);
    test(1e-9, 9);  // terminating digit is '1' exactly
    test(1e-9, 8);  // termination before '1' digit, output all zeroes
    test(1e-9, 7);

    test(8e-9, 20);
    test(8e-9, 10);
    test(8e-9, 9);  // terminating digit is '8' exactly
    test(8e-9, 8);  // termination before '8' digit, rounds up
    test(8e-9, 7);  // rounds to zero

    test(9e-9, 20);
    test(9e-9, 10);
    test(9e-9, 9);  // terminating digit is '9' exactly
    test(9e-9, 8);  // termination before '9' digit, rounds up
    test(9e-9, 7);  // rounds to zero

    test(0.999999999, 20);
    test(0.999999999, 10);
    test(0.999999999, 9);  // terminates at last '9'
    test(0.999999999, 8);  // round up and wrap
    test(0.999999999, 7);  // round up and wrap
}

try {
    fixedTest();
} catch (e) {
    print(e);
}
