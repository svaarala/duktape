/*===
2 inf at 1024
3 inf at 647
4 inf at 512
5 inf at 442
6 inf at 397
7 inf at 365
8 inf at 342
9 inf at 324
10 inf at 309
11 inf at 297
12 inf at 286
13 inf at 277
14 inf at 269
15 inf at 263
16 inf at 256
17 inf at 251
18 inf at 246
19 inf at 242
20 inf at 237
21 inf at 234
22 inf at 230
23 inf at 227
24 inf at 224
25 inf at 221
26 inf at 218
27 inf at 216
28 inf at 214
29 inf at 211
30 inf at 209
31 inf at 207
32 inf at 205
33 inf at 203
34 inf at 202
35 inf at 200
36 inf at 199
===*/

/* Exponent overflow and underflow have special handling in the parser:
 * exponent values way above overflow or below underflow for a non-zero
 * significand have an early reject, to keep bigint size bounded.
 *
 * Here, test that exponent values above and below these ranges behave
 * properly.  Concretely, we try values with exponent in the range
 * [-2000,2000] for all radix values and ensure that conversiona are
 * correct and no unexpected errors are detected.  Numbers are built
 * explicitly without exponent notation to allow testing of non-decimal
 * numbers.
 */

var zero1k = ''
while (zero1k.length < 1000) {
    zero1k += '0';
}

function getZeros(n) {
    var t = [];
    while (n >= 1000) {
        t.push(zero1k);
        n -= 1000;
    }
    if (n > 0) {
        t.push(zero1k.substring(0, n));
        n--;
    }
    return t.join('');
}

function buildNumber(exp) {
    if (exp > 0) {
        return '1' + getZeros(exp);
    } else if (exp < 0) {
        return '0.' + getZeros(-exp - 1) + '1';
    } else {
        return '0';
    }
}

function expOverflowUnderflowTest() {
    var i, str, radix, t, val;

    if (this.__engine__ === 'rhino') {
        throw new Error("Rhino forever loops on some exponents, so skip test on Rhino");
    }

    for (radix = 2; radix <= 36; radix++) {
        // These limits are enough for binary radix too: they cover the
        // valid range (about +/- 1000) and go over enough to hit the
        // early reject limit too

        t = [];
        for (i = -2000; i <= 2000; i++) {
            str = buildNumber(i);
            // XXX: no standard way to parse fractions in other radix values,
            // add a custom API to test these properly?
            if (i < 0) {
                if (radix != 10) { continue; }
                val = parseFloat(str);
            } else {
                val = parseInt(str, radix);
            }

            // we don't care the absolute value (separate parsing tests cover
            // that), just whether the result is infinite or not
            t.push(isFinite(val) ? 'f' : 'i');
            if (t[t.length - 2] === 'f' && t[t.length - 1] === 'i') {
                print(radix, 'inf at', i);
            }
        }

        //print(radix, t.join(''));
    }
}

try {
    expOverflowUnderflowTest();
} catch (e) {
    print(e);
}
