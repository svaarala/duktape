/*---
{
    "slow": true
}
---*/

var ms_day = 24 * 60 * 60 * 1000;

var days_in_month_noleap = [
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
];

var days_in_month_leap = [
    31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
];

function isLeapYear(x) {
    if (Math.floor(x) !== x) {
       throw new Error('unexpected input: ' + x);
    }

    if ((x % 4) != 0) {
        return false;
    }
    if ((x % 100) != 0) {
        return true;
    }
    if ((x % 400) != 0) {
        return false;
    }
    return true;
}

/*===
brute force conversion test
===*/

/* Brute force back and forth conversion test for each start-of-month
 * for years in the range [-10000, 10000].
 *
 * Testing the entire E5 valid range (+/- 100e6 days from Jan 1, 1970)
 * takes ~4 seconds V8, but with Duktape running under valgrind it
 * takes way too long, so we only test a shorter range here and rely
 * on spot checks in other parts of the range.
 */

function bruteForceConversionTest() {
    var timeval;
    var year, month;
    var dt;
    var isLeap;
    var dayNumber;
    var dayLimit;

    function msg(x) {
        return 'brute force conversion test failed: ' +
               x + ' (year=' + year + ', month=' + month +
               ', dayNumber=' + dayNumber + ', timeval=' + timeval + ')';
    }

    function testYearAndMonth() {
        // Date from UTC breakdown; note that using Date.UTC() is a bad idea
        // because it assumes years 0-99 mean years 1900-1999.
        //
        // dt = new Date(Date.UTC(year, month /*zero-based month*/, 1, 0, 0, 0, 0));
        //
        // Also note that if you set fields individually, and any intermediate
        // timestamp is out of 'E5 range' (i.e. will cause TimeClip() to return
        // a NaN), the internal time value will become NaN and won't recover to a
        // valid value even if another setter call would make the time valid again.
        // To avoid this issue, set year last.

        /* This would be one alternative (slower):
        dt = new Date(0);
        dt.setUTCMilliseconds(0);
        dt.setUTCSeconds(0);
        dt.setUTCMinutes(0);
        dt.setUTCHours(0);
        dt.setUTCDate(1);
        dt.setUTCMonth(month);
        dt.setUTCFullYear(year);
        */

        dt = new Date(Date.UTC(year, month /*zero-based month*/, 1, 0, 0, 0, 0));
        dt.setUTCFullYear(year);  // fix Date if year in [0,99]

        if (dt.getTime() !== timeval) {
            throw new Error(msg('getTime !== timeVal: ' + dt.getTime() + ' vs ' + timeval));
        }
        if (dt.getUTCFullYear() !== year) {
            throw new Error(msg('getUTCFullYear !== year'));
        }
        if (dt.getUTCMonth() !== month) {
            throw new Error(msg('getUTCMonth !== month'));
        }
        if (dt.getUTCDate() !== 1) {
            throw new Error(msg('getUTCDate !== 1'));
        }
    }

    timeval = 0;
    dayNumber = 0;
    dayLimit = 2933263;  // Mon, 01 Jan 10001 00:00:00 GMT
    year = 1970; month = 0;

    for (;;) {
        if (dayNumber > dayLimit) {
            break;
        }

        testYearAndMonth();

        isLeap = isLeapYear(year);
        timeval += (isLeap ? days_in_month_leap : days_in_month_noleap)[month] * ms_day;
        dayNumber += (isLeap ? days_in_month_leap : days_in_month_noleap)[month];

        month++;
        if (month == 12) {
            year++;
            month = 0;
        }
    }

    timeval = 0;
    dayNumber = 0;
    dayLimit = -4371953;  // Sat, 01 Jan -10000 00:00:00 GMT
    year = 1970; month = 0;
    for (;;) {
        if (dayNumber < dayLimit) {
            break;
        }

        testYearAndMonth();

        month--;
        if (month == -1) {
            year--;
            month = 11;
        }

        isLeap = isLeapYear(year);
        timeval -= (isLeap ? days_in_month_leap : days_in_month_noleap)[month] * ms_day;
        dayNumber -= (isLeap ? days_in_month_leap : days_in_month_noleap)[month];
    }
}

try {
    print('brute force conversion test');
    bruteForceConversionTest();
} catch (e) {
    print(e.name);
}
