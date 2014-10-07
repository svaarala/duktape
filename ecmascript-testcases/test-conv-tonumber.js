/*
 *  ToNumber() (E5 Section 9.3).
 *
 *  Postfix increment changes its LHS, put returns ToNumber(oldValue) as the
 *  expression value.  Use this get ToNumber() indirectly.
 */

function tonumber(x) {
    var tmp = x;
    return tmp++;
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
    var num = tonumber(x);
    print(num, zeroSign(num));
}

/*===
NaN nz
0 pos
1 nz
0 pos
-1 nz
0 neg
0 pos
1 nz
NaN nz
Infinity nz
-Infinity nz
===*/

test(undefined);
test(null);
test(true);
test(false);
test(-1);
test(-0);
test(+0);
test(1);
test(NaN);
test(Number.POSITIVE_INFINITY);
test(Number.NEGATIVE_INFINITY);

/*===
-1 nz
0 neg
0 pos
0 pos
1 nz
-Infinity nz
Infinity nz
Infinity nz
3735928559 nz
===*/

/* String to number */

test("-1");
test("-0");
test("+0");
test("0");
test("1");

test("-Infinity");
test("+Infinity");
test("Infinity");

test("0xdeadbeef");

/* XXX: octal */

/* XXX: object coercion */
