/*===
0
32
3802
-1439089
32
32
-1439089
-1439089
-719474
-719474
===*/

/* MakeDay() is not exposed directly, but setUTCFullYear() will call MakeDay()
 * with parameters given in the call.  It will then combine the result with
 * MakeDate(), keeping the time-within-day same as before.  By using 00:00:00
 * as the time, the day number can then be computed easily from the result.
 *
 * However: the arguments to MakeDay() will be ToNumber() coerced by
 * setUTCFullYear(), and the result will go through TimeClip().
 */

// Note: month is zero-based, day is one-based
function MakeDay(year, month, day) {
    var d;
    var res;

    d = new Date(0);
    d.setUTCFullYear(year, month, day);
    res = d.getTime() / (24 * 60 * 60 * 1000);
    if (Math.floor(res) !== res) {
        throw new Error('MakeDay() result not a multiple of days');
    }
    return res;
}

try {
    // simple tests
    print(MakeDay(1970, 0, 1));
    print(MakeDay(1970, 1, 2));

    // components don't need to be normalized on input
    // 123 = 10 years + 3 months
    print(MakeDay(1970, 123, 60));

    // components may also be negative
    print(MakeDay(-1970, -1, -2));

    // components are ToInteger() rounded, which rounds towards zero
    print(MakeDay(1970.1, 1.1, 2.1));
    print(MakeDay(1970.9, 1.9, 2.9));
    print(MakeDay(-1970.1, -1.1, -2.1));
    print(MakeDay(-1970.9, -1.9, -2.9));

    // Components even outside valid ecmascript range are OK as long as
    // the result will be within Ecmascript range.  Here day and month
    // should cancel out, leaving year zero, day 55.
    print(MakeDay(100e10, -1200e10, 55));
    print(MakeDay(0, 0, 55));

    // XXX: rounding errors?  MakeTime() is required to use Ecmascript
    // (IEEE double) arithmetic.  These never occur unless extremely
    // unnormalized values are used.
} catch (e) {
    print(e.name);
}
