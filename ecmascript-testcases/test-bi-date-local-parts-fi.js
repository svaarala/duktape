/*
 *  When parsing a timestamp from local time parts (year, month, etc) the
 *  implementation needs to figure out the UTC time corresponding to the
 *  local parts and then convert the UTC time to an Ecmascript time value.
 *
 *  The concrete conversion algorithm tries to figure out the UTC-to-local
 *  offset so that it can be taken into account in the Ecmascript time value.
 *  There's a practical problem in this conversion: DUK__GET_LOCAL_TZOFFSET()
 *  provides a UTC-to-local-time offset for an input time value which is
 *  essentially a *UTC* timestamp.  However, we don't know the UTC time yet
 *  (we're trying to compute it).
 *
 *  The current solution in Duktape's duk_bi_date.c is to compute an
 *  approximate time value by treating the local parts like UTC parts,
 *  yielding a time value that is off by 12 hours maximum.  The local time
 *  offset is computed for this approximate timestamp and the offset is
 *  then applied to arrive at an approximation of the UTC time which is at
 *  most 1 hour off.  This process is repeated a few times until the time
 *  offset no longer changes.
 */

// Test is custom because of custom local time formatting
/*---
{
    "custom": true
}
---*/

/*===
finnish locale test
1396141199000 2014-03-30T00:59:59.000Z 2014-03-30 02:59:59.000+02:00
1396141199000 2014-03-30T00:59:59.000Z 2014-03-30 02:59:59.000+02:00
1396141200000 2014-03-30T01:00:00.000Z 2014-03-30 04:00:00.000+03:00
1414281599000 2014-10-25T23:59:59.000Z 2014-10-26 02:59:59.000+03:00
1414285200000 2014-10-26T01:00:00.000Z 2014-10-26 03:00:00.000+02:00
===*/

function finnishLocaleTest() {
    var d;

    // 2014-03-30 02:59:59 Finnish time is still EET (GMT+0200)
    d = new Date(2014, 2, 30, 2, 59, 59);
    print(d.getTime(), d.toISOString(), d.toString());

    // 2014-03-30 03:59:59 Finnish time never normally occurs (one steps
    // from the non-DST 02:59:59 to the DST 04:00:00).  This case thus
    // has an ambiguous result, but we now test for behavior matching
    // V8 and Rhino.
    d = new Date(2014, 2, 30, 3, 59, 59);
    print(d.getTime(), d.toISOString(), d.toString());

    // 2014-03-30 04:00:00 Finnish time is EEST (GMT+0300)
    d = new Date(2014, 2, 30, 4, 0, 0);
    print(d.getTime(), d.toISOString(), d.toString());

    // 2014-10-26 02:59:59 Finnish time is still EEST (GMT+0300)
    d = new Date(2014, 9, 26, 2, 59, 59);
    print(d.getTime(), d.toISOString(), d.toString());

    // 2014-10-26 03:00:00 Finnish time is EET (GMT+0200)
    d = new Date(2014, 9, 26, 3, 0, 0);
    print(d.getTime(), d.toISOString(), d.toString());
}

print('finnish locale test');

try {
    finnishLocaleTest();
} catch (e) {
    print(e);
}
