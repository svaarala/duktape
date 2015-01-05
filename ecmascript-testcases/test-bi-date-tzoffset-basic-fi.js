/*
 *  Minimal tzoffset test for Finnish locale.
 */

// Custom test because of locale specific timestamp outputs
/*---
{
    "custom": true
}
---*/

/*===
tzo for current time is correct
1325368800000 2011-12-31T22:00:00.000Z 2012-01-01 00:00:00.000+02:00 -120
1343768400000 2012-07-31T21:00:00.000Z 2012-08-01 00:00:00.000+03:00 -180
1396141199000 2014-03-30T00:59:59.000Z 2014-03-30 02:59:59.000+02:00 -120
1396141200000 2014-03-30T01:00:00.000Z 2014-03-30 04:00:00.000+03:00 -180
1396141199000 2014-03-30T00:59:59.000Z 2014-03-30 02:59:59.000+02:00 -120
1396141200000 2014-03-30T01:00:00.000Z 2014-03-30 04:00:00.000+03:00 -180
1414281599000 2014-10-25T23:59:59.000Z 2014-10-26 02:59:59.000+03:00 -180
1414285200000 2014-10-26T01:00:00.000Z 2014-10-26 03:00:00.000+02:00 -120
===*/

/*
Rhino:
tzo for current time is correct
1325368800000 2011-12-31T22:00:00.000Z Sun Jan 01 2012 00:00:00 GMT+0200 (EET) -120
1343768400000 2012-07-31T21:00:00.000Z Wed Aug 01 2012 00:00:00 GMT+0300 (EEST) -180
1396141199000 2014-03-30T00:59:59.000Z Sun Mar 30 2014 02:59:59 GMT+0200 (EET) -120
1396141200000 2014-03-30T01:00:00.000Z Sun Mar 30 2014 04:00:00 GMT+0300 (EEST) -180
1396141199000 2014-03-30T00:59:59.000Z Sun Mar 30 2014 02:59:59 GMT+0200 (EET) -120
1396141200000 2014-03-30T01:00:00.000Z Sun Mar 30 2014 04:00:00 GMT+0300 (EEST) -180
1414281599000 2014-10-25T23:59:59.000Z Sun Oct 26 2014 02:59:59 GMT+0300 (EEST) -180
1414285200000 2014-10-26T01:00:00.000Z Sun Oct 26 2014 03:00:00 GMT+0200 (EET) -120
*/

/*
V8:
tzo for current time is correct
1325368800000 2011-12-31T22:00:00.000Z Sun Jan 01 2012 00:00:00 GMT+0200 (EET) -120
1343768400000 2012-07-31T21:00:00.000Z Wed Aug 01 2012 00:00:00 GMT+0300 (EEST) -180
1396141199000 2014-03-30T00:59:59.000Z Sun Mar 30 2014 02:59:59 GMT+0200 (EET) -120
1396141200000 2014-03-30T01:00:00.000Z Sun Mar 30 2014 04:00:00 GMT+0300 (EEST) -180
1396141199000 2014-03-30T00:59:59.000Z Sun Mar 30 2014 02:59:59 GMT+0200 (EET) -120
1396141200000 2014-03-30T01:00:00.000Z Sun Mar 30 2014 04:00:00 GMT+0300 (EEST) -180
1414281599000 2014-10-25T23:59:59.000Z Sun Oct 26 2014 02:59:59 GMT+0300 (EEST) -180
1414285200000 2014-10-26T01:00:00.000Z Sun Oct 26 2014 03:00:00 GMT+0200 (EET) -120
*/

function finnishTzoffsetBasicTest() {
    var d, tzo;

    function dump(d) {
        print(d.getTime(), d.toISOString(), d.toString(), d.getTimezoneOffset());
    }

    // Current timezone offset is +02:00 or +03:00
    d = new Date();
    tzo = d.getTimezoneOffset();
    if (tzo == -120 || tzo == -180) {
        print('tzo for current time is correct');
    } else {
        print('tzo for current time is incorrect: ' + tzo);
    }

    // Jan 1 timezone offset is +02:00
    d = new Date(2012, 0, 1, 0, 0, 0);
    dump(d);

    // Aug 1 timezone offset is +03:00
    d = new Date(2012, 7, 1, 0, 0, 0);
    dump(d);

    // 2014-03-30 02:59:59 Finnish time is still EET (GMT+0200)
    d = new Date(2014, 2, 30, 2, 59, 59);
    dump(d);
    d = new Date(d.getTime() + 1000);
    dump(d);

    // 2014-03-30 03:NN:NN Finnish time doesn't really exist, as one
    // jumps from 02:59:59 (no DST) to 04:00:00 (DST) in local time:
    //
    // > d = new Date(Date.parse('2014-03-30T00:59:59.000Z'))
    // Sun Mar 30 2014 02:59:59 GMT+0200 (EET)
    // > d = new Date(d.getTime() + 1000)
    // Sun Mar 30 2014 04:00:00 GMT+0300 (EEST)
    //
    // So, resolving times of the form 03:NN:NN is ambiguous.  Rhino
    // and V8 seem to favor the earlier time instant (at least in this
    // case):
    //
    // > d = new Date(2014, 2, 30, 3, 59, 59);
    // Sun Mar 30 2014 02:59:59 GMT+0200 (EET)
    // > d.getTime()
    // 1396141199000
    //
    // Duktape used to iterate for the tzoffset without arriving at a
    // conclusive result, like this:
    //
    // [D] duk_bi_date.c:1209 (duk__get_timeval_from_dparts): tzoffset iteration, i=0, tzoff=0, tzoffnew=10800
    // [D] duk_bi_date.c:1209 (duk__get_timeval_from_dparts): tzoffset iteration, i=1, tzoff=10800, tzoffnew=7200
    // [D] duk_bi_date.c:1209 (duk__get_timeval_from_dparts): tzoffset iteration, i=2, tzoff=7200, tzoffnew=10800
    // [D] duk_bi_date.c:1209 (duk__get_timeval_from_dparts): tzoffset iteration, i=3, tzoff=10800, tzoffnew=7200
    // [D] duk_bi_date.c:1216 (duk__get_timeval_from_dparts): tzoffset iteration, tzoff=7200
    //
    // This was changed to favor a higher value in a two-value loop so that
    // 10800 gets selected and behavior matches Rhino/V8, at least in this
    // instance.  Not sure if this is generically correct:
    //
    // [D] duk_bi_date.c:1214 (duk__get_timeval_from_dparts): tzoffset iteration, i=0, tzoff=10800, tzoffprev1=0 tzoffprev2=999999999
    // [D] duk_bi_date.c:1214 (duk__get_timeval_from_dparts): tzoffset iteration, i=1, tzoff=7200, tzoffprev1=10800 tzoffprev2=0
    // [D] duk_bi_date.c:1214 (duk__get_timeval_from_dparts): tzoffset iteration, i=2, tzoff=10800, tzoffprev1=7200 tzoffprev2=10800
    // [D] duk_bi_date.c:1226 (duk__get_timeval_from_dparts): tzoffset iteration two-value cycle, i=2, tzoff=10800, tzoffprev1=7200, tzoffprev2=10800
    // [D] duk_bi_date.c:1233 (duk__get_timeval_from_dparts): tzoffset iteration, tzoff=10800

    d = new Date(2014, 2, 30, 3, 59, 59);
    dump(d);

    // 2014-03-30 04:00:00 Finnish time is EEST (GMT+0300)
    d = new Date(2014, 2, 30, 4, 0, 0);
    dump(d);

    // 2014-10-26 02:59:59 Finnish time is still EEST (GMT+0300)
    d = new Date(2014, 9, 26, 2, 59, 59);
    dump(d);

    // 2014-10-26 03:00:00 Finnish time is EET (GMT+0200)
    d = new Date(2014, 9, 26, 3, 0, 0);
    dump(d);
}

try {
    finnishTzoffsetBasicTest();
} catch (e) {
    print(e);
}
