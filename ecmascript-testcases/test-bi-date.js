/*
 *  Date objects (E5 Section 15.9).
 */

/*===
1973-03-03T09:46:40.123Z
100000000123
NaN
===*/

function test() {
    var d;

    // Date objects have no instance properties, internal primitive
    // value preserves argument but fractional milliseconds are coerced
    // with ToInteger (towards zero).  Negative zero handling may differ
    // between implementations and is tested by test-bi-date-timeclip.js.
    d = new Date(100000000123.75);
    print(d.toISOString());
    print(d.valueOf());

    // prototype [[PrimitiveValue]] is NaN
    print(Date.prototype.valueOf());
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
