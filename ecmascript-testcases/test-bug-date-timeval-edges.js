/*
 *  Date corner case bugs found through test262
 *
 *  Ecmascript E5.1 Section 15.9.1.1
 *
 *      The actual range of times supported by ECMAScript Date objects
 *      is slightly smaller: exactly -100,000,000 days to 100,000,000
 *      days measured relative to midnight at the beginning of 01 January,
 *      1970 UTC. This gives a range of 8,640,000,000,000,000 milliseconds
 *      to either side of 01 January, 1970 UTC.
 */

/*===
test1
8639999996399999
+275760-09-12T22:59:59.999Z
test2
8639999996400000
+275760-09-12T23:00:00.000Z
test3
-8640000000000001 RangeError
-8640000000000000 -271821-04-20T00:00:00.000Z
-8639999999999999 -271821-04-20T00:00:00.001Z
-1 1969-12-31T23:59:59.999Z
0 1970-01-01T00:00:00.000Z
0 1970-01-01T00:00:00.000Z
1 1970-01-01T00:00:00.001Z
8639999999999999 +275760-09-12T23:59:59.999Z
8640000000000000 +275760-09-13T00:00:00.000Z
8640000000000001 RangeError
===*/

/* Adapted from: ch15/15.9/15.9.5/15.9.5.43/15.9.5.43-0-11.
 *
 * This test used to fail because tzoffset computation was avoided (with
 * result replaced with zero) when a temporary timevalue was outside the
 * strict Ecmascript range.  This was fixed by adding a 24h leeway to that
 * range check in duk__get_local_tzoffset().
 */
function test1() {
    var tzMin;
    var d;

    tzMin= new Date().getTimezoneOffset() * (-1);
    //print('tzMin:', tzMin);

    d = new Date(1970,            // year
                 0,               // month (Jan)
                 100000001,       // days: 100M + 1 (one day over Ecmascript maximum)
                 0,               // hour
                 0 + tzMin - 60,  // minutes: one hour backwards from day 101M UTC
                 0,               // seconds
                 -1);             // milliseconds

    print(d.valueOf());
    print(d.toISOString());
}

/* Adapted from: ch15/15.9/15.9.5/15.9.5.43/15.9.5.43-0-12 */
function test2() {
    var tzMin;
    var d;

    tzMin= new Date().getTimezoneOffset() * (-1);
    //print('tzMin:', tzMin);

    d = new Date(1970,            // year
                 0,               // month (Jan)
                 100000001,       // days: 100M + 1 (one day over Ecmascript maximum)
                 0,               // hour
                 0 + tzMin - 60,  // minutes: one hour backwards from day 101M UTC
                 0,               // seconds
                 0);              // milliseconds

    print(d.valueOf());
    print(d.toISOString());
}

function test3() {
    /* toString(), which coerces to local time, caused an assert failure for
     * similar reasons as with test1() and test2().  When converting an
     * Ecmascript time value to local time, the implementation adds a local
     * time offset before generating the parts (year, month, etc).  This
     * temporary time value can be just outside Ecmascript range even if the
     * original UTC time value is within the range.  A +/- 24h leeway was
     * added to the assert in duk__timeval_to_parts() to fix the assert error.
     */

    [ -8640e12 - 1,   // invalid
      -8640e12,       // smallest valid
      -8640e12 + 1,
      -1,
      -0,
      +0,
      +1,
      8640e12 - 1,
      8640e12,        // largest valid
      8640e12 + 1 ]   // invalid
    .forEach(function (x) {
        var d, ign;
        try {
            d = new Date(x);
            print(x, d.toISOString());
            ign = d.toString();  // this caused an assert failure once
        } catch (e) {
            print(x, e.name);
        }
    });
}

try {
    print('test1');
    test1();
} catch (e) {
    print(e.stack || e);
}

try {
    print('test2');
    test2();
} catch (e) {
    print(e.stack || e);
}

try {
    print('test3');
    test3();
} catch (e) {
    print(e.stack || e);
}
