/*
 *  String.prototype.repeat()
 */

/*===
xxxxxxxxxx
xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
50
51966 97 98 99 2047 51966 97 98 99 2047 51966 97 98 99 2047 51966 97 98 99 2047 51966 97 98 99 2047 51966 97 98 99 2047 51966 97 98 99 2047 51966 97 98 99 2047 51966 97 98 99 2047 51966 97 98 99 2047
0
0
0
0
0
5
RangeError
0
RangeError
RangeError
0
246
0
3000000
0
100
200
300
400
500
600
700
800
900
RangeError
all done
===*/

function test() {
    var res;
    var i, j, k, n;

    // basic usage
    res = 'x'.repeat(10);
    print(res);

    // chaining
    res = 'x'.repeat(2).repeat(5).repeat(10);
    print(res);

    // unicode
    res = '\ucafeabc\u07ff'.repeat(10);
    print(res.length);
    print(Array.prototype.map.call(res, function (v) { return v.charCodeAt(0); }).join(' '));

    res = ''.repeat(1e9);
    print(res.length);
    try {
        res = ''.repeat(1e15);  // allowed: as long as ToInteger() coerce check succeeds and result is small enough
        print(res.length);
    } catch (e) {
        print(e.name);
    }
    res = 'xxxx'.repeat(0);
    print(res.length);
    res = 'x'.repeat(1e6).repeat(0);
    print(res.length);
    res = ''.repeat(0);
    print(res.length);

    // ToInteger coercions fractions towards zero.
    res = 'x'.repeat(5.9);  // allowed
    print(res.length);

    // ToInteger(+inf) is +inf, but E6 Section 21.1.3.13 step 7 rejects it
    // even when the input size is zero.
    try {
        res = ''.repeat(1/0);
        print(res.length);
    } catch (e) {
        print(e.name);
    }

    // negative counts are always rejected, but the value is truncated towards
    // zero and negative zero is allowed
    res = ''.repeat(-0.9);  // allowed
    print(res.length);
    try {
        res = ''.repeat(-1);  // not allowed
        print(res.length);
    } catch (e) {
        print(e.name);
    }
    try {
        res = ''.repeat(-1/0);  // not allowed
        print(res.length);
    } catch (e) {
        print(e.name);
    }

    // NaN coerces to +0 in ToInteger() so allowed
    res = 'xxx'.repeat(0/0);
    print(res.length);

    // Repeat count is ToNumber() coerced, so e.g. '123' works.
    res = 'xy'.repeat('123.1');
    print(res.length);

    // And anything that doesn't parse as a number is equivalent to 0.
    res = 'xy'.repeat('dummy');
    print(res.length);

    // large input, small repeat count
    res = 'x'.repeat(1e6).repeat(3);
    print(res.length);

    // brute force some counts and lengths
    var inputs = [
        'x',
        'xy',
        'xyz',
        'xyzz',
        'xyzzy\ucafe\u1234\u07ffabcdefghijklmnopqrstuvwxyz'
    ];
    for (i = 0; i < 1e3; i++) {
        if ((i % 100) == 0) { print(i); }
        for (j = 0; j < 5; j++) {
            res = inputs[j].repeat(i);
            n = inputs[j].length;
            if (res.length !== n * i) {
                throw new Error('length mismatch: ' + i + ',' + j);
            }
            for (k = 0; k < res.length; k++) {
                if (res.charCodeAt(k) !== inputs[j].charCodeAt(k % n)) {
                    throw new Error('charcode mismatch: ' + k + ',' + i + ',' + j);
                }
            }
        }
    }

    // string length overflow, assume for now it happens at least around 1024G
    try {
        res = 'x'.repeat(256).repeat(256).repeat(256).repeat(256).repeat(256);
    } catch (e) {
        print(e.name);
    }

    print('all done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
