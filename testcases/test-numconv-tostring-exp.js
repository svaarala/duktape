/*===
1e+2
8e+2
1e+3
1.23e+4
9.88e+4
1.00e+5
1e+4
1e-1
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
    test(99999, 2);  // ronuds up, over first digit

    test(12345, undefined);
    test(0.1, undefined);
}

try {
    expTest();
} catch (e) {
    print(e);
}

