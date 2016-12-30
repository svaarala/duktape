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
-1 nz
0 neg
0 pos
0 pos
1 nz
17 nz
19 nz
-Infinity nz
Infinity nz
Infinity nz
3735928559 nz
3735928559 nz
3735928559 nz
NaN nz
===*/

/* String to number */

test('-1');
test('   -1    ');  // whitespace is trimmed
test('-0');
test('+0');
test('0');
test('1');
test('000017');  // lead zeroes allowed, not interpreted as octal!
test('000019');

test('  -Infinity');
test(' +Infinity');
test('   Infinity');

test('0xdeadbeef');
test('0Xdeadbeef');
test('    \n0xdeadbeef\n');
test('    \n0xdeadbeefg\n');  // trailing garbage not allowed -> NaN

/*===
83 nz
83 nz
83 nz
83 nz
NaN nz
17 nz
17 nz
17 nz
17 nz
NaN nz
===*/

/* In ES2015 0o123, and 0b10001 are recognized.  Whitespace is also allowed.
 * Note that whitespace is trimmed so these forms cannot be recognized
 * by just peeking at the first few characters.
 */

test('0o123');
test('0O123');
test('\n0o123  \t');
test('\n0o000000123  \t');
test('\n0o000000123  x\t');
test('0b10001');
test('0B10001');
test('\t0b10001  \r\n');
test('\t0b0000010001  \r\n');
test('\t0b00000000000100012  \r\n');  // '2' is garbage to binary

/* XXX: object coercion */
