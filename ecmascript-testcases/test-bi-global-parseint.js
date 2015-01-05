/*
 *  parseInt() tests
 *
 *  parseInt() must have "mathematically exact" results for any supported
 *  power of two radix (2, 4, 8, 16, 32).  For instance, parsing the base-8
 *  representation of 2**256 must yield the exact IEEE double value.  This
 *  is easy for base two radixes.  However, the same property is required
 *  also for base 10 (though not for other bases), which is tricky.
 */

var WHITESPACE_CODEPOINTS = [
    // from WhiteSpace production
    0x0009,    // <TAB>
    0x000B,    // <VT>
    0x000C,    // <FF>
    0x0020,    // <SP>
    0x00A0,    // <NBSP>
    0xFEFF,    // <BOM>

    // WhiteSpace production also has <USP>, which means any other Unicode
    // space separator (category Zs), which needs to be checked from (up to
    // date) Unicode data.  The WhiteSpace-Z.txt file, created as part of
    // the build, currently contains (duplicates eliminated):

//  0x0020,    // 0020;SPACE;Zs;0;WS;;;;;N;;;;;
//  0x00A0,    // 00A0;NO-BREAK SPACE;Zs;0;CS;<noBreak> 0020;;;;N;NON-BREAKING SPACE;;;;
    0x1680,    // 1680;OGHAM SPACE MARK;Zs;0;WS;;;;;N;;;;;
    0x180E,    // 180E;MONGOLIAN VOWEL SEPARATOR;Zs;0;WS;;;;;N;;;;;
    0x2000,    // 2000;EN QUAD;Zs;0;WS;2002;;;;N;;;;;
    0x2001,    // 2001;EM QUAD;Zs;0;WS;2003;;;;N;;;;;
    0x2002,    // 2002;EN SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
    0x2003,    // 2003;EM SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
    0x2004,    // 2004;THREE-PER-EM SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
    0x2005,    // 2005;FOUR-PER-EM SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
    0x2006,    // 2006;SIX-PER-EM SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
    0x2007,    // 2007;FIGURE SPACE;Zs;0;WS;<noBreak> 0020;;;;N;;;;;
    0x2008,    // 2008;PUNCTUATION SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
    0x2009,    // 2009;THIN SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
    0x200A,    // 200A;HAIR SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
//  0x2028,    // 2028;LINE SEPARATOR;Zl;0;WS;;;;;N;;;;;
//  0x2029,    // 2029;PARAGRAPH SEPARATOR;Zp;0;B;;;;;N;;;;;
    0x202F,    // 202F;NARROW NO-BREAK SPACE;Zs;0;CS;<noBreak> 0020;;;;N;;;;;
    0x205F,    // 205F;MEDIUM MATHEMATICAL SPACE;Zs;0;WS;<compat> 0020;;;;N;;;;;
    0x3000,    // 3000;IDEOGRAPHIC SPACE;Zs;0;WS;<wide> 0020;;;;N;;;;;

    // from LineTerminator production
    0x000a,    // <LF>
    0x000d,    // <CR>
    0x2028,    // <LS>
    0x2029,    // <PS>
];

// indirect eval -> this is bound to the global object, E5 Section 10.4.2, step 1.a.
var g = (function () { var e = eval; return e('this'); } )();

/*===
basic tests
NaN
123
51966
65
58798832
===*/

/* parseInt: some very basic cases */

print('basic tests');

try {
    print(g.parseInt(''));
    print(g.parseInt('  123'));
    print(g.parseInt('cafe', 16));
    print(g.parseInt('01000001', 2));  // smallest supported radix
    print(g.parseInt('z09gw', 36));    // largest supported radix
} catch (e) {
    print(e.name);
}

/*===
coercion
toString()
123
===*/

/* parseInt coercion: ToString() followed by string parsing. */

print('coercion');

try {
    print(g.parseInt(
        { toString: function() { print('toString()'); return '123'; },
          valueOf: function() { print('valueOf()'); return 123; } }
    ));
} catch (e) {
    print(e.name);
}

/*===
whitespace strip test
321
123
123
===*/

/* parseInt white space stripping.
 *
 * StrWhiteSpaceChar is WhiteSpace or LineTerminator; WHITESPACE_CODEPOINTS
 * lists the codepoints.
 */

print('whitespace strip test');

function whiteSpaceStripTest() {
    var t = [];
    var i;
    var txt;

    // a simple test with only ASCII whitespace
    txt = '\u0009\u000b\u000c\u0020\u00a0\u000a\u000d321.0';
    print(g.parseInt(txt));

    // txt will contain every whitespace codepoint followed by the number
    for (i = 0; i < WHITESPACE_CODEPOINTS.length; i++) {
        t.push(String.fromCharCode(WHITESPACE_CODEPOINTS[i]));
    }
    t.push('123.0');
    txt = t.join('');
    print(g.parseInt(txt));

    // try with trailing garbage
    print(g.parseInt(txt + 'xyz'));
}

try {
    whiteSpaceStripTest();
} catch (e) {
    print(e.name);
}

/*===
trailing garbage
123
123
123
123
123
123
-123
-123
-123
-128
128
-57005
4660
===*/

/* Trailing garbage is ignored. */

print('trailing garbage');

function trailingGarbageTest() {
    // decimal
    print(g.parseInt('123xxx'));
    print(g.parseInt('123\uffff'));
    print(g.parseInt('123\u0000'));
    print(g.parseInt('+123xxx'));
    print(g.parseInt('+123\uffff'));
    print(g.parseInt(' +123\u0000'));
    print(g.parseInt(' -123xxx'));
    print(g.parseInt('  -123\uffff'));
    print(g.parseInt('  -123\u0000'));

    // a few tests in another radix
    print(g.parseInt('  -10000000xxx', 2));
    print(g.parseInt('  10000000xxx', 2));

    // whitespace is also "garbage"
    print(g.parseInt('  -0xdead beefxxx', 16));
    print(g.parseInt('   1234 5678xxx', 16));
}

try {
    trailingGarbageTest();
} catch (e) {
    print(e.name);
}

/*===
fractions
12345678
12345678
12345678
-12345678
-12345678
-12345678
-128
128
===*/

/* Fractions (decimal or otherwise) as created by e.g. Number.prototype.toString()
 * are ignored when using parseInt; they are treated as garbage.
 */

print('fractions');

function fractionsTest() {
    // decimal
    print(g.parseInt('12345678.9'));
    print(g.parseInt('12345678.1'));
    print(g.parseInt(' +12345678.9'));
    print(g.parseInt(' -12345678.1'));
    print(g.parseInt('  -12345678.9'));
    print(g.parseInt('  -12345678.1'));

    // a few tests in another radix
    print(g.parseInt('  -10000000.001', 2));
    print(g.parseInt('  10000000.100', 2));
}

try {
    fractionsTest();
} catch (e) {
    print(e.name);
}

/*===
radix tests
NaN
8
1000
46656
NaN
1000
1000
1000
1000
1000
1000
46656
46656
NaN
8
1000
46656
NaN
NaN
8
1000
46656
NaN
42875
42875
46656
===*/

/* Valid radix range is 2 to 36, and is coerced with ToInt32.  Note that
 * because ToInt32() uses a modulo-based coercion, integer radix N is the
 * same as radix N + k*2^32.  Rounding is towards zero.
 *
 * R = 0 has special handling, it defaults R to 10 (step 9).
 */

print('radix tests');

function radixTest() {
    var txt = "1000";
    var tp32 = 256 * 256 * 256 * 256;

    // basic integer range
    print(g.parseInt(txt, 1));
    print(g.parseInt(txt, 2));
    print(g.parseInt(txt, 10));
    print(g.parseInt(txt, 36));  // 1*36*36*36
    print(g.parseInt(txt, 37));

    // not given, undefined, 0 radix are the same and interpreted as radix 10
    print(g.parseInt(txt));
    print(g.parseInt(txt, undefined));
    print(g.parseInt(txt, 0));
    print(g.parseInt(txt, 10));

    // round towards zero
    print(g.parseInt(txt, 10.1));
    print(g.parseInt(txt, 10.9));
    print(g.parseInt(txt, 36.1));
    print(g.parseInt(txt, 36.9));

    // modulo effect
    print(g.parseInt(txt, 1 + tp32));
    print(g.parseInt(txt, 2 + tp32));
    print(g.parseInt(txt, 10 + tp32));
    print(g.parseInt(txt, 36 + tp32));
    print(g.parseInt(txt, 37 + tp32));
    print(g.parseInt(txt, 1 - tp32));
    print(g.parseInt(txt, 2 - tp32));
    print(g.parseInt(txt, 10 - tp32));
    print(g.parseInt(txt, 36 - tp32));
    print(g.parseInt(txt, 37 - tp32));

    // fractional radixes work differently because of round-towards-zero.
    // positive values round downwards before modulo computation; negative
    // values effectively round upwards
    print(g.parseInt(txt, 35.9 + tp32));  // 4294967331.9 -> 4294967331 -> 35 (after -2^32)
    print(g.parseInt(txt, 35.9));
    print(g.parseInt(txt, 35.9 - tp32));  // -4294967260.1 -> -4294967260 -> 36 (after +2^32)
}

try {
    radixTest();
} catch (e) {
    print(e.name);
}

/*===
radix 16
51966
-51966
51966
-51966
-51966
51966
-51966
51966
0
0
0
0
===*/

/* parseInt radix is defaulted to 16 if the string begins with "0x" or "0X".
 * The prefix "0x" or "0X" is permitted (only) if radix is 16.
 */

print('radix 16');

function testRadix16(x, r, arg_count) {
    var t;

    try {
        if (arg_count == 1) {
            t = g.parseInt(x);
        } else {
            t = g.parseInt(x, r);
        }
        print(t);
    } catch (e) {
        print(e);
    }
}

function radix16Test() {
    testRadix16('0xcafe', undefined, 1);
    testRadix16('-0Xcafe', undefined, 1);
    testRadix16('   0xCAfe', undefined, 1);  // whitespace + case variation
    testRadix16('   -0XCAfe', undefined, 1);

    testRadix16('-0xcafe', 16);
    testRadix16('0Xcafe', 16);
    testRadix16('   -0xCAfe', 16.5);
    testRadix16('   0XCAfe', 16.5);

    /* The expected result here is not obvious.  Let's consider the
     * first case.
     *
     * In step 6, R will be 15, which causes us to execute step 8.b,
     * i.e. stripPrefix will be false.  Step 10 will then be skipped,
     * and the '0x' prefix should NOT cause an automatic radix 16
     * conversion.
     *
     * Parsing in base 15, the first character will be a valid digit
     * (zero), while the second character will be garbage and terminate
     * parsing.  The result should, thus be 0.
     *
     * (However, at least V8/Rhino will parse these as hexadecimal.)
     */

    testRadix16('0xcafe', 15);
    testRadix16('0Xcafe', 15);
    testRadix16('0x1234', 10);
    testRadix16('0X1234', 10);
}

try {
    radix16Test();
} catch (e) {
    print(e.name);
}

/*===
signed values
123
-123
291
-291
66
-66
===*/

/* Signed values; test negative values in other radixes especially. */

print('signed values');

function signedValueTest() {
    print(g.parseInt(' +123'));
    print(g.parseInt(' -123'));
    print(g.parseInt(' +0x123'));
    print(g.parseInt(' -0X123'));
    print(g.parseInt(' +123', 7));
    print(g.parseInt(' -123', 7));
}

try {
    signedValueTest();
} catch (e) {
    print(e.name);
}

/*===
leading zeroes
123
123
-123
129
129
-129
57005
57005
-57005
668
668
-668
===*/

/* Leading zeroes.
 *
 * Note that V8 and Rhino use a leading zero (not followed by 'x' or 'X') to
 * indicate an automatic radix 8 (octal).  This doesn't seem spec compliant,
 * so test against this for now.
 */

/* XXX: change Duktape behavior to match V8 and Rhino for octal? */

print('leading zeroes');

function leadingZeroTest() {
    // V8 will yield +/- 83 for this
    print(g.parseInt('000123'));
    print(g.parseInt('+000123'));
    print(g.parseInt('-000123'));

    // V8 will yield +/- 10 for this (012 = 10 octal, 9 is garbage)
    print(g.parseInt('000129'));
    print(g.parseInt('+000129'));
    print(g.parseInt('-000129'));

    print(g.parseInt('0x0000dead'));
    print(g.parseInt('+0x0000dead'));
    print(g.parseInt('-0x0000dead'));

    print(g.parseInt('00001234', 8));
    print(g.parseInt('+00001234', 8));
    print(g.parseInt('-00001234', 8));
}

try {
    leadingZeroTest();
} catch (e) {
    print(e.name);
}

/*===
numbers below/above 2**53
(testdump)
9007199254740984
9007199254740985
9007199254740986
9007199254740987
9007199254740988
9007199254740989
9007199254740990
9007199254740991
9007199254740992
9007199254740992
9007199254740994
9007199254740996
9007199254740996
9007199254740996
9007199254740998
9007199254741000
9007199254741000
(parse decimals)
9007199254740992
9007199254740992
9007199254740994
9007199254740996
9007199254740996
9007199254740996
9007199254740998
9007199254741000
9007199254741000
(parse hex)
9007199254740992
9007199254740992
9007199254740994
9007199254740996
9007199254740996
9007199254740996
9007199254740998
9007199254741000
9007199254741000
(parse radix 2)
9007199254740992
9007199254740992
9007199254740994
9007199254740996
9007199254740996
9007199254740996
9007199254740998
9007199254741000
9007199254741000
(parse radix 4)
9007199254740992
9007199254740992
9007199254740994
9007199254740996
9007199254740996
9007199254740996
9007199254740998
9007199254741000
9007199254741000
(parse radix 8)
9007199254740992
9007199254740992
9007199254740994
9007199254740996
9007199254740996
9007199254740996
9007199254740998
9007199254741000
9007199254741000
(parse radix 32)
9007199254740992
9007199254740992
9007199254740994
9007199254740996
9007199254740996
9007199254740996
9007199254740998
9007199254741000
9007199254741000
(parse radix 36)
9007199254740992
9007199254740992
9007199254740994
9007199254740996
9007199254740996
9007199254740996
9007199254740998
9007199254741000
9007199254741000
===*/

/* Numbers close to just below and above the IEEE double range (53 bits). */

print('numbers below/above 2**53');

function numbersNear53BitsTest() {
    var two_to_53 = 65536*65536*65536*2*2*2*2*2;  // 2**53
    var i;

    // demonstrate how behavior after 2**53 changes
    // XXX: elaborate on the rounding behavior
    print('(testdump)');
    for (i = -8; i <= 8; i++) {
        print(two_to_53 + i);
    }

    print('(parse decimals)');
    print(g.parseInt('9007199254740992'));
    print(g.parseInt('9007199254740993'));
    print(g.parseInt('9007199254740994'));
    print(g.parseInt('9007199254740995'));
    print(g.parseInt('9007199254740996'));
    print(g.parseInt('9007199254740997'));
    print(g.parseInt('9007199254740998'));
    print(g.parseInt('9007199254740999'));
    print(g.parseInt('9007199254741000'));

    print('(parse hex)');
    print(g.parseInt('0x20000000000000'));
    print(g.parseInt('0x20000000000001'));
    print(g.parseInt('0x20000000000002'));
    print(g.parseInt('0x20000000000003'));
    print(g.parseInt('0x20000000000004'));
    print(g.parseInt('0x20000000000005'));
    print(g.parseInt('0x20000000000006'));
    print(g.parseInt('0x20000000000007'));
    print(g.parseInt('0x20000000000008'));

    print('(parse radix 2)');
    print(g.parseInt('100000000000000000000000000000000000000000000000000000', 2));
    print(g.parseInt('100000000000000000000000000000000000000000000000000001', 2));
    print(g.parseInt('100000000000000000000000000000000000000000000000000010', 2));
    print(g.parseInt('100000000000000000000000000000000000000000000000000011', 2));
    print(g.parseInt('100000000000000000000000000000000000000000000000000100', 2));
    print(g.parseInt('100000000000000000000000000000000000000000000000000101', 2));
    print(g.parseInt('100000000000000000000000000000000000000000000000000110', 2));
    print(g.parseInt('100000000000000000000000000000000000000000000000000111', 2));
    print(g.parseInt('100000000000000000000000000000000000000000000000001000', 2));

    print('(parse radix 4)');
    print(g.parseInt('200000000000000000000000000', 4));
    print(g.parseInt('200000000000000000000000001', 4));
    print(g.parseInt('200000000000000000000000002', 4));
    print(g.parseInt('200000000000000000000000003', 4));
    print(g.parseInt('200000000000000000000000010', 4));
    print(g.parseInt('200000000000000000000000011', 4));
    print(g.parseInt('200000000000000000000000012', 4));
    print(g.parseInt('200000000000000000000000013', 4));
    print(g.parseInt('200000000000000000000000020', 4));

    print('(parse radix 8)');
    print(g.parseInt('400000000000000000', 8));
    print(g.parseInt('400000000000000001', 8));
    print(g.parseInt('400000000000000002', 8));
    print(g.parseInt('400000000000000003', 8));
    print(g.parseInt('400000000000000004', 8));
    print(g.parseInt('400000000000000005', 8));
    print(g.parseInt('400000000000000006', 8));
    print(g.parseInt('400000000000000007', 8));
    print(g.parseInt('400000000000000010', 8));

    print('(parse radix 32)');
    print(g.parseInt('80000000000', 32));
    print(g.parseInt('80000000001', 32));
    print(g.parseInt('80000000002', 32));
    print(g.parseInt('80000000003', 32));
    print(g.parseInt('80000000004', 32));
    print(g.parseInt('80000000005', 32));
    print(g.parseInt('80000000006', 32));
    print(g.parseInt('80000000007', 32));
    print(g.parseInt('80000000008', 32));

    print('(parse radix 36)');
    print(g.parseInt('2gosa7pa2gw', 36));
    print(g.parseInt('2gosa7pa2gx', 36));
    print(g.parseInt('2gosa7pa2gy', 36));
    print(g.parseInt('2gosa7pa2gz', 36));
    print(g.parseInt('2gosa7pa2h0', 36));
    print(g.parseInt('2gosa7pa2h1', 36));
    print(g.parseInt('2gosa7pa2h2', 36));
    print(g.parseInt('2gosa7pa2h3', 36));
    print(g.parseInt('2gosa7pa2h4', 36));

    /* Parsing results for radix 3 are not required to be exact, so
     * printing these results would result in a custom test.  V8 and
     * Rhino also have different results for the commented out radix 3
     * test below in practice, too.
     */

    /*
    print('(parse radix 3)');
    print(g.parseInt('1121202011211211122211100012101112', 3));
    print(g.parseInt('1121202011211211122211100012101120', 3));
    print(g.parseInt('1121202011211211122211100012101121', 3));
    print(g.parseInt('1121202011211211122211100012101122', 3));
    print(g.parseInt('1121202011211211122211100012101200', 3));
    print(g.parseInt('1121202011211211122211100012101201', 3));
    print(g.parseInt('1121202011211211122211100012101202', 3));
    print(g.parseInt('1121202011211211122211100012101210', 3));
    print(g.parseInt('1121202011211211122211100012101211', 3));
    */
}

try {
    numbersNear53BitsTest();
} catch (e) {
    print(e.name);
}

/*===
large number test
2 3.402823669209385e+38 true
3 diff ok true
4 3.402823669209385e+38 true
5 diff ok true
6 diff ok true
7 diff ok true
8 3.402823669209385e+38 true
9 diff ok true
10 3.402823669209385e+38 true
11 diff ok true
12 diff ok true
13 diff ok true
14 diff ok true
15 diff ok true
16 3.402823669209385e+38 true
17 diff ok true
18 diff ok true
19 diff ok true
20 diff ok true
21 diff ok true
22 diff ok true
23 diff ok true
24 diff ok true
25 diff ok true
26 diff ok true
27 diff ok true
28 diff ok true
29 diff ok true
30 diff ok true
31 diff ok true
32 3.402823669209385e+38 true
33 diff ok true
34 diff ok true
35 diff ok true
36 diff ok true
===*/

/* Numbers much higher than 53 bits.
 *
 * Check 2**128 in all supported radixes.
 */

/*
digits = '0123456789abcdefghijklmnopqrstuvwxyz';
print('// len(digits) = %d' % len(digits))

def tobase(x,n):
  t = []
  while x != 0:
    t.append(digits[x % n])
    x = x / n
  t.reverse()
  return ''.join(t)

print('[')
for i in xrange(2, 37):
  print('{ base: %d, str: "%s" },' % (i, tobase(2**128, i)))

print(']')
*/

print('large number test');

function largeNumberTest() {
    var inputs = [
        { base: 2, str: "100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000" },
        { base: 3, str: "202201102121002021012000211012011021221022212021111001022110211020010021100121011" },
        { base: 4, str: "10000000000000000000000000000000000000000000000000000000000000000" },
        { base: 5, str: "11031110441201303134210404233413032443021130230130231311" },
        { base: 6, str: "23053353530155550541354043543542243325553444410304" },
        { base: 7, str: "3115512162124626343001006330151620356026315304" },
        { base: 8, str: "4000000000000000000000000000000000000000000" },
        { base: 9, str: "22642532235024164257285244038424203240534" },
        { base: 10, str: "340282366920938463463374607431768211456" },
        { base: 11, str: "1000a504186892265432152aa27a7929366353" },
        { base: 12, str: "5916b64b41143526a777873841863a6a6994" },
        { base: 13, str: "47168c9c477c94ba75a2bc735955c7aa139" },
        { base: 14, str: "51a45cb9506962a31983dac25409715d04" },
        { base: 15, str: "7d491176809c28848a561186d4857d321" },
        { base: 16, str: "100000000000000000000000000000000" },
        { base: 17, str: "279078gb8485d7b72e2ag3c08ed3g121" },
        { base: 18, str: "78a399ccdeb5bd6ha3184c0fh64da64" },
        { base: 19, str: "1910510fd19aig25hc6g5gebeb98h84" },
        { base: 20, str: "66f609c19456i2h147iga2g17b68cg" },
        { base: 21, str: "1b71cc7c703ijd4g3k82ff6keb3c04" },
        { base: 22, str: "8h8b5gheh7legf73fb0had7bhd3de" },
        { base: 23, str: "2c59d9lld38jeh6fgh5m42j82lfdd" },
        { base: 24, str: "iamei9lfd1i5k10n7fnfn25b3kag" },
        { base: 25, str: "6365o71fgjb44dj83en26fd1fd86" },
        { base: 26, str: "23745echihn4jcil7jec2kd69a1m" },
        { base: 27, str: "kjbg2750m547p8n7d18cm6379g4" },
        { base: 28, str: "81a71cgjeb6cjo7odb65d7icrl4" },
        { base: 29, str: "36l6q70ega3gd8s9ag8rce14rap" },
        { base: 30, str: "1a4p5qh8koob2e2gknbn3jbm88g" },
        { base: 31, str: "hleoq9ui363gg16lmp8srn1je8" },
        { base: 32, str: "80000000000000000000000000" },
        { base: 33, str: "3nakotlgi17b10fs1825j5cf1p" },
        { base: 34, str: "1ppi6nphe0ckbctn7qwp6qrh9i" },
        { base: 35, str: "try5wbbiprfp7r727m0oyq2wb" },
        { base: 36, str: "f5lxx1zz5pnorynqglhzmsp34" },
    ];
    var results = [];
    var i;
    var diff;

    // these are required to be exact by the specification: since 2**128 can be represented
    // exactly, the result must be exact and equal for all of these
    var required_exact = { '2': true, '4': true, '8': true, '16': true, '32': true, '10': true };

    for (i = 0; i < inputs.length; i++) {
        results.push(g.parseInt(inputs[i].str, inputs[i].base));

        if (required_exact[inputs[i].base]) {
            // Assume base 2 is correct and compare everything against that
            // for those radixes for which results are required to be bit
            // exact.

            print(inputs[i].base, results[i], results[i] === results[0]);
        } else {
            // Fon't print out the results for other bases, as they don't
            // necessarily match (even in practice).
            //
            // For 2**128, given mantissa is 53 bits (one bit implicit),
            // expect around some multiple of 2**(128-53) = 2**75 =
            // 3.777893186295716e+22 of error.  Limit allows over 10-fold
            // error now.

            diff = results[i] - results[0];
            print(inputs[i].base, 'diff ok', Math.abs(diff) < 4e23);

            //print(inputs[i].base, results[i], results[i] === results[0]);
        }
    }
}

try {
    largeNumberTest();
} catch (e) {
    print(e.name);
}

/*===
digit cases
3735928559
1047601316294262
===*/

/* Upper and lowercase digits. */

print('digit cases');

function digitCaseTest() {
    print(g.parseInt('0xdeADbEeF'));
    print(g.parseInt('aBcDeFgGhI', 36));
}

try {
    digitCaseTest();
} catch (e) {
    print(e.name);
}
