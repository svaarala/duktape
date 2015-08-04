/*===
0.00
1.00
1.00
0.9999
0.99990
1.23457e+7
1.234568e+7
12345678
12345678.0
8.76543e+7
8.765432e+7
87654321
87654321.0
5.556e+4
55555
4.444e+4
44444
1.000e+5
99999
0
0
0.0
0.00
0
0
0.0
0.00
===*/

function precisionTest() {
    function test(x, n) {
        print(new Number(x).toPrecision(n));
    }

    test(0, 3);
    test(1, 3);

    test(0.9999, 3);
    test(0.9999, 4);
    test(0.9999, 5);

    test(12345678, 6);  // rounds up
    test(12345678, 7);  // rounds up
    test(12345678, 8);
    test(12345678, 9);

    test(87654321, 6);  // rounds down
    test(87654321, 7);  // rounds down
    test(87654321, 8);
    test(87654321, 9);

    test(55555, 4);  // rounds up
    test(55555, 5);
    test(44444, 4);  // rounds down
    test(44444, 5);

    test(99999, 4);  // rounds up, carries
    test(99999, 5);

    // NB: precision=0 is a RangeError
    test(+0, undefined);  // -> ToString()
    test(+0, 1);
    test(+0, 2);
    test(+0, 3);
    test(-0, undefined);  // -> ToString()
    test(-0, 1);
    test(-0, 2);
    test(-0, 3);
}

try {
    precisionTest();
} catch (e) {
    print(e);
}
