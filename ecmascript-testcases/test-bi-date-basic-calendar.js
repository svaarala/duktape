var ms_day = 24 * 60 * 60 * 1000;

/*===
time value basic tests
1970-01-01T00:00:00.000Z
1970-01-01T00:00:01.000Z
1970-01-01T00:00:01.123Z
1970-01-01T01:00:00.000Z
1970-01-02T00:00:00.000Z
===*/

print('time value basic tests');

try {
    print(new Date(0).toISOString());
    print(new Date(1000).toISOString());
    print(new Date(1123).toISOString());
    print(new Date(3600e3).toISOString());
    print(new Date(86400e3).toISOString());
} catch (e) {
    print(e.name);
}

/*===
leap year tests
1903 1 28
1903 2 1
1904 1 28
1904 1 29
1904 2 1
1900 1 28
1900 2 1
2000 1 28
2000 1 29
2000 2 1
===*/

print('leap year tests');

function leapYearTests() {
    var d;

    // 1903 is not a leap year
    d = new Date(Date.UTC(1903, 1, 28));  // Feb 28, 1903
    print(d.getUTCFullYear(), d.getUTCMonth(), d.getUTCDate());
    d = new Date(Date.UTC(1903, 1, 29));  // wraps to Mar 1, 1903
    print(d.getUTCFullYear(), d.getUTCMonth(), d.getUTCDate());

    // 1904 is a leap year (4-year rule)
    d = new Date(Date.UTC(1904, 1, 28));  // Feb 28, 1904
    print(d.getUTCFullYear(), d.getUTCMonth(), d.getUTCDate());
    d = new Date(Date.UTC(1904, 1, 29));  // Feb 29, 1904
    print(d.getUTCFullYear(), d.getUTCMonth(), d.getUTCDate());
    d = new Date(Date.UTC(1904, 1, 30));  // wraps to Mar 1, 1904
    print(d.getUTCFullYear(), d.getUTCMonth(), d.getUTCDate());

    // 1900 is not a leap year (100-year rule)
    d = new Date(Date.UTC(1900, 1, 28));  // Feb 28, 1900
    print(d.getUTCFullYear(), d.getUTCMonth(), d.getUTCDate());
    d = new Date(Date.UTC(1900, 1, 29));  // wraps to Mar 1, 1900
    print(d.getUTCFullYear(), d.getUTCMonth(), d.getUTCDate());

    // 2000 is a leap year (400-year rule)
    d = new Date(Date.UTC(2000, 1, 28));  // Feb 28, 2000
    print(d.getUTCFullYear(), d.getUTCMonth(), d.getUTCDate());
    d = new Date(Date.UTC(2000, 1, 29));  // Feb 29, 2000
    print(d.getUTCFullYear(), d.getUTCMonth(), d.getUTCDate());
    d = new Date(Date.UTC(2000, 1, 30));  // wraps to Mar 1, 2000
    print(d.getUTCFullYear(), d.getUTCMonth(), d.getUTCDate());
}

try {
    leapYearTests();
} catch (e) {
    print(e.name);
}

/*===
month day counts
1903 31 28 31 30 31 30 31 31 30 31 30 31
1904 31 29 31 30 31 30 31 31 30 31 30 31
1900 31 28 31 30 31 30 31 31 30 31 30 31
2000 31 29 31 30 31 30 31 31 30 31 30 31
===*/

print('month day counts');

function getMonthDayCount(year, month) {
    var dt;
    var res = 0;

    dt = new Date(Date.UTC(year, month, 1));

    while (dt.getUTCMonth() === month) {
        res++;
        dt = new Date(dt.getTime() + ms_day);
    }

    return res;
}

function testMonthDays() {
    var years = [ 1903 /* non-leap */, 1904 /* leap */, 1900 /* non-leap */, 2000 /* leap */ ];
    var year;
    var month;
    var i;
    var tmp;

    for (i = 0; i < years.length; i++) {
        year = years[i];
        tmp = [];
        for (month = 0; month < 12; month++) {
            tmp.push(getMonthDayCount(year, month));
        }
        print(year, tmp.join(' '));
    }
}

try {
    testMonthDays();
} catch (e) {
    print(e.name);
}

/*===
week day tests
4
5 6 0 1 2 3 4 5 6 0
===*/

print('week day tests');

function testWeekDays() {
    var dt;
    var i;
    var tmp = [];

    dt = new Date(0);  // Jan 1, 1970 is a thursday (= 4)
    print(dt.getUTCDay());

    for (i = 0; i < 10; i++) {
        dt = new Date(dt.getTime() + ms_day);
        tmp.push(dt.getUTCDay());
    }

    print(tmp.join(' '));
}

try {
    testWeekDays();
} catch (e) {
    print(e.name);
}
