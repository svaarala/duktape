/*===
1e+308
-1e+308
Infinity
-Infinity
1e+300
-1e+300
1e+100
-1e+100
1e+100
-1e+100
1e+100
-1e+100
===*/

/* Both Rhino and V8 will parse these extreme cases, where the mantissa
 * is way above the IEEE range but the exponent brings the result down
 * to IEEE range.  The problem with parsing these is avoiding arbitrarily
 * large intermediate big numbers.
 *
 * Since the current number conversion algorithm uses statically allocated
 * bigints (to minimize memory usage, memory churn, etc), there is currently
 * a limit on the kinds of numbers that can be parsed.
 */

function limitsTest() {
    var i, t;

    function build(n) {
        var t = [ 1 ];
        for (i = 0; i < n; i++) {
            t.push(0);
            if (t.length > 1000) {
                /* This is needed to get around current (default) valstack limit */
                t = [ t.join('') ];
            }
        }
        return t.join('');
    }

    // +/- 1e308 in explicit form

    t = build(308);
    print(Number(t));
    print(Number('-' + t));

    // +/- 1e309 in explicit form -> wrap to Infinity

    t = build(309);
    print(Number(t));
    print(Number('-' + t));

    // mantissa is above 1e308 but exponent brings the whole number
    // down to finite range

    t = build(400);
    t += 'e-100';   // effectively, number is 1e300
    print(Number(t));
    print(Number('-' + t));

    // even more extreme cases

    t = build(10000);
    t += 'e-9900';  // effectively, number is 1e100
    print(Number(t));
    print(Number('-' + t));

    t = build(100000);
    t += 'e-99900';  // effectively, number is 1e100
    print(Number(t));
    print(Number('-' + t));

    t = build(1000000);
    t += 'e-999900';  // effectively, number is 1e100
    print(Number(t));
    print(Number('-' + t));

    /* Note: there is a current implementation limit for maximum exponent.
     * There is a separate test for its presence.
     */
}

try {
    limitsTest();
} catch (e) {
    print(e);
}
