/*
 *  Symbol behavior in operators.
 */

/*@include util-symbol.js@*/

/*===
symbol operator
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
TypeError
true
false
true
false
true
false
false
false
===*/

function symbolOperatorTest() {
    var s1, s2;
    var t;

    function test(fn) {
        try {
            print(fn());
        } catch (e) {
            //print(e.stack);
            //print(e);
            print(e.name);
        }
    }

    // In basic arithmetic Symbols get either coerced to strings or numbers
    // which causes TypeError.
    s1 = Symbol('foo');
    s2 = Symbol('foo');
    test(function () { return s1 + s2; });
    test(function () { return s1 - s2; });
    test(function () { return s1 * s2; });
    test(function () { return s1 / s2; });
    test(function () { return +s2; });
    test(function () { return -s2; });

    // Addition is a bit special in its coercion behavior so test a few
    // type combinations.
    test(function () { return 'foo' + s1; });
    test(function () { return s1 + 'foo'; });
    test(function () { return 123 + s1; });
    test(function () { return s1 + 123; });

    // Comparison: arguments are ToPrimitive() coerced.  Symbols go on to
    // be ToNumber() coerced which ultimately fails the comparison.  Also
    // test for some mixed combinations.
    test(function () { return s1 < s2; });
    test(function () { return s1 <= s2; });
    test(function () { return s1 > s2; });
    test(function () { return s1 >= s2; });
    test(function () { return 'foo' < s1; });
    test(function () { return 'foo' <= s1; });
    test(function () { return 'foo' > s1; });
    test(function () { return 'foo' >= s1; });
    test(function () { return s1 < 'foo'; });
    test(function () { return s1 <= 'foo'; });
    test(function () { return s1 > 'foo'; });
    test(function () { return s1 >= 'foo'; });
    test(function () { return 123 < s1; });
    test(function () { return 123 <= s1; });
    test(function () { return 123 > s1; });
    test(function () { return 123 >= s1; });
    test(function () { return s1 < 123; });
    test(function () { return s1 <= 123; });
    test(function () { return s1 > 123; });
    test(function () { return s1 >= 123; });

    // In strict equality Symbols compare by their "heap pointer".

    // In non-strict equality Symbol-to-Object comparison uses ToPrimitive()
    // on the object which allows Symbol values and Symbol objects to be
    // compared as true.  However, Symbol object to Symbol object comparison
    // is false even if the underlying symbol value is the same.
    test(function () { return s1 == s1; });
    test(function () { return s1 == s2; });
    test(function () { return s1 == Object(s1); });
    test(function () { return s1 == Object(s2); });
    test(function () { return Object(s1) == s1; });
    test(function () { return Object(s1) == s2; });
    test(function () { return Object(s1) == Object(s1); });
    test(function () { return Object(s1) == Object(s2); });
}

try {
    print('symbol operator');
    symbolOperatorTest();
} catch (e) {
    print(e.stack || e);
}
