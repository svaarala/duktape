/*
 *  Exponentiation and exponentiation assignment operators (E7 Section 12.6)
 */

/*===
exponentiation operator
1024
4
256
65536
8192
65536
===*/

function expOperatorTest() {
    var x, y, z, w;

    x = 2;
    y = 8;
    z = 4;
    w = 16;
    print(2 ** 10);
    print(x ** 2);
    print(2 ** y);
    print(w ** z);
    print(x * y ** z);  // '**' outranks '*'
    print(x ** x ** z);  // right associative
}

try {
    print("exponentiation operator");
    expOperatorTest();
} catch (e) {
    print(e.stack || e);
}

/*===
exponentiation assignment
1000
16
===*/

function expAssignmentTest() {
    var x, y;

    x = 10;
    x **= 3;
    print(x);
    y = 2;
    y **= y **= 2;
    print(y);
}

try {
    print("exponentiation assignment");
    expAssignmentTest();
} catch (e) {
    print(e.stack || e);
}


/*===
special cases
NaN
1
1
NaN
Infinity
0
NaN
NaN
0
Infinity
Infinity
0
-Infinity
Infinity
-0
0
0
Infinity
-0
0
-Infinity
Infinity
NaN
===*/

function expSpecialCasesTest() {
    // Use temporaries to ensure the calculations are done at runtime and not
    // prematurely inlined by the compiler.
    var one = 1, minusOne = -1, two = 2, minusTwo = -2, half = 0.5;
    var zero = 0, minusZero = -0;
    var inf = Infinity, minusInf = -Infinity;
    var nan = NaN;

    // Allow 0 and -0 to be differentiated in the output.
    function printNumber(value) {
        if (value == 0 && (1 / value) < 0) {
            print("-0");
        } else {
            print(value);
        }
    }

    // E7 Section 12.7.3.4
    printNumber(two ** nan);
    printNumber(nan ** zero);
    printNumber(nan ** minusZero);
    printNumber(nan ** one);
    printNumber(two ** inf);
    printNumber(two ** minusInf);
    printNumber(one ** inf);
    printNumber(minusOne ** minusInf);
    printNumber(half ** inf);
    printNumber(half ** minusInf);
    printNumber(inf ** one);
    printNumber(inf ** minusOne);
    printNumber(minusInf ** one);
    printNumber(minusInf ** two);
    printNumber(minusInf ** minusOne);
    printNumber(minusInf ** minusTwo);
    printNumber(zero ** one);
    printNumber(zero ** minusOne);
    printNumber(minusZero ** one);
    printNumber(minusZero ** two);
    printNumber(minusZero ** minusOne);
    printNumber(minusZero ** minusTwo);
    printNumber(minusOne ** half);
}

try {
    print("special cases")
    expSpecialCasesTest();
} catch (e) {
    print(e.stack || e);
}
