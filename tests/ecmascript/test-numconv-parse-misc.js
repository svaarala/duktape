/*===
0.1
0.1
1
NaN
100000
1
1
1
Infinity
-Infinity
1461559270678
-1461559270678
0.1
SyntaxError
SyntaxError
SyntaxError
100000
SyntaxError
SyntaxError
SyntaxError
Infinity
-Infinity
1e+23
10000000000000000000
100000000000000000000
1e+21
1e+22
10000000000000000000
100000000000000000000
1e+21
1e+22
10000000000000000000
100000000000000000000
1e+21
1e+22
0.0001
0.0001
0.0001
0.0001
0.0001
===*/

/* Misc tests from specification and own documentation. */

function miscTest() {
    function pFloat(x) {
        try {
            print(parseFloat(x));
        } catch (e) {
            print(e.name);
        }
    }

    function pInt(x, radix) {
        try {
            print(parseInt(x, radix));
        } catch (e) {
            print(e.name);
        }
    }

    function pJSON(x) {
        try {
            print(JSON.parse(x));
        } catch (e) {
            print(e.name);
        }
    }

    // parseFloat() allows fractions without leading integer digit
    pFloat('0.1');
    pFloat('.1');

    // parseFloat() also allows decimal point without fractions,
    // but does not allow a decimal point without a leading integer
    // digit AND without fraction digits
    pFloat('1.');
    pFloat('.');

    // Exponent must have at least one digit.  However, if the exponent
    // is malformed, pFloat() finds the shortest valid prefix, and parses
    // the invalid cases as '1'!
    pFloat('1e5');
    pFloat('1e');
    pFloat('1e+');
    pFloat('1e-');

    // parseFloat() parses 'Infinity'
    pFloat('Infinity');
    pFloat('-Infinity');

    // parseInt() does not; in suitable radix 'Infinity' is a valid number
    pInt('Infinity', 36);
    pInt('-Infinity', 36);

    // JSON.parse does not allow fractions without leading digits, or
    // decimal point without at least one fractional digit
    pJSON('0.1');  // OK
    pJSON('.1');
    pJSON('1.');
    pJSON('.');

    // JSON.parse requires at least one digit in exponent
    pJSON('1e5');
    pJSON('1e');
    pJSON('1e+');
    pJSON('1e-');

    // JSON.parse() preserves sign
    print(1/JSON.parse('0'));  // -> +inf
    print(1/JSON.parse('-0')); // -> -inf

    // V8 prints "1e+23", some Duktape version prints something else
    // (a separate bug testcase opened)
    print(parseInt('0000100000000000000000000000', 10));

    // various positions for precision digits
    pFloat('10000000000000000000');
    pFloat('100000000000000000000');
    pFloat('1000000000000000000000');
    pFloat('10000000000000000000000');
    pFloat('00010000000000000000000');
    pFloat('000100000000000000000000');
    pFloat('0001000000000000000000000');
    pFloat('00010000000000000000000000');
    pFloat('00000000000000000000000010000000000000000000');
    pFloat('000000000000000000000000100000000000000000000');
    pFloat('0000000000000000000000001000000000000000000000');
    pFloat('00000000000000000000000010000000000000000000000');
    pFloat('0.0001');
    pFloat('0.00010000000000000000000');
    pFloat('0.000100000000000000000000');
    pFloat('0.0001000000000000000000000');
    pFloat('0.00010000000000000000000000');
}

try {
    miscTest();
} catch (e) {
    print(e);
}
