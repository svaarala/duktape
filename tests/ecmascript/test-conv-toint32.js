/*
 *  ToInt32() (E5 Section 9.5).
 *
 *  Indirect testing using '<<' (E5 Section 11.7.1): "x << 0" is equivalent
 *  to ToInt32().  ToInt32() first coerces with ToNumber(), so that is noted
 *  in the cases.
 */

function toint32(x) {
    return x << 0;
}

function zeroSign(x) {
    if (x !== 0) {
        return 'nz';
    }
    if (1 / x > 0) {
        return 'pos';
    } else {
        return 'neg';
    }
}

function test(x) {
    var t = toint32(x);
    print(t, zeroSign(t));
}

/*===
0 pos
===*/

// ToNumber(undefined) -> NaN -> +0
test(undefined);

/*===
0 pos
===*/

// ToNumber(null) -> +0 -> +0
test(null);

/*===
1 nz
0 pos
===*/

// ToNumber(true) -> 1 -> 1
test(true);

// Tonumber(false) -> +0 -> +0
test(false);

/*===
0 pos
0 pos
0 pos
0 pos
0 pos
===*/

// ToNumber(NaN) -> NaN -> +0
test(NaN);

// ToNumber(+0) -> +0 -> +0
test(+0);

// ToNumber(-0) -> -0 -> +0
test(-0);

// ToNumber(+Infinity) -> +Infinity -> +0
test(Number.POSITIVE_INFINITY);

// ToNumber(-Infinity) -> -Infinity -> +0
test(Number.NEGATIVE_INFINITY);

/*===
123 nz
-123 nz
0 pos
0 pos
0 pos
0 pos
0 pos
0 pos
0 pos
1410065408 nz
===*/

/* ToNumber() string coercion, E5 Section 9.3.1 */

test("123");
test("-123");
test("+0");
test("-0");
test("Infinity");
test("+Infinity");
test("-Infinity");

test("NaN");  // "NaN" does not parse -> results in NaN number -> +0
test("NaY");  // same case

test("1e10");  // larger than 32-bit, fits in 53 bits of double

/* XXX: object coercion */

/*===
0 pos
1 nz
-1 nz
2 nz
-1 nz
0 pos
2 nz
===*/

/* A couple of modulo tests */

test(4294967296);
test(4294967297);
test(-1);
test(-4294967294);        // +2
test(9007199254740991);   // (2^53 - 1) % (2^32) --> 4294967295 --> -1
test(9007199254740992);   // (2^53) % (2^32) --> 0
test(9007199254740994);   // (2^53 + 2) % (2^32) --> 2  (Note: 2^53+1 not representable)

/*===
3 nz
-3 nz
===*/

/* Rounding
 *
 * Positive numbers: x -> floor(x)     e.g. 3.4 -> 3
 * Negative numbers: -x -> -floor(x)   e.g. -3.4 -> -3  (not -4)
 */

test(3.4);   // -> 3 -> 3
test(-3.4);  // -> -3 -> -3
