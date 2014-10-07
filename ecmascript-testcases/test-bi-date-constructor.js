function dateToComponentString(dt) {
    return [
               dt.getUTCFullYear(),
               dt.getUTCMonth(),
               dt.getUTCDate(),
               dt.getUTCHours(),
               dt.getUTCMinutes(),
               dt.getUTCSeconds(),
               dt.getUTCMilliseconds()
           ].join(' ');
}

function utcDiffAtTime(dt) {
    var d1;

    // This is not exactly accurate but good enough for testing:
    // take the UTC components and use them as local ones and see
    // how much difference that makes.
    d1 = new Date(dt.getUTCFullYear(),
                  dt.getUTCMonth(),
                  dt.getUTCDate(),
                  dt.getUTCHours(),
                  dt.getUTCMinutes(),
                  dt.getUTCSeconds(),
                  dt.getUTCMilliseconds());

    // Positive offset = local time is ahead of UTC (e.g. EET is
    // +7200e3 or +10800e3
    return dt.getTime() - d1.getTime();
}

/*===
date constructor as function
string
true
string
true
string
true
string
true
string
true
===*/

/* Date constructor called as a function: all arguments are ignored, equivalent
 * to 'new Date().toString()'.
 */

print('date constructor as function');

function dateConstructorAsFunctionTest() {
    var now = new Date().getTime();
    var t;
    var obj;

    // toString() result should parse back to a Date, so we check it parses
    // back close to current time
    t = Date();
    print(typeof t);
    print(Math.abs(Date.parse(t) - now) < 5000);

    // same test, but checks that arguments are properly ignored as required
    // in E5.1 Section 15.9.2.1.
    t = Date(2030, 1, 2, 3, 4, 5, 6);
    print(typeof t);
    print(Math.abs(Date.parse(t) - now) < 5000);

    // even invalid values must be ignored
    t = Date(-1e12, 1e12, 2e12, 3e12, 4e12, 5e12, 6e12);
    print(typeof t);
    print(Math.abs(Date.parse(t) - now) < 5000);

    // even values of invalid type
    t = Date('foo', 'bar', 'quux');
    print(typeof t);
    print(Math.abs(Date.parse(t) - now) < 5000);

    // because no coercion happens, no side effects should trigger
    obj = { toString: function() { print('toString'); return 'toString' },
            valueOf: function() { print('valueOf'); return 'valueOf' } };
    t = Date(obj, obj, obj, obj, obj, obj, obj);
    print(typeof t);
    print(Math.abs(Date.parse(t) - now) < 5000);
}

try {
    dateConstructorAsFunctionTest();
} catch (e) {
    print(e.name);
}

/*===
date constructor tests
true
1970 0 1 0 1 0 0
NaN NaN NaN NaN NaN NaN NaN
Invalid Date
2013 3 1 0 0 0 0
2013 1 2 3 4 5 6
-200000 1 2 3 4 5 6
200000 1 2 3 4 5 6
-1 1 2 3 4 5 6
1900 1 2 3 4 5 6
1999 1 2 3 4 5 6
100 1 2 3 4 5 6
NaN NaN NaN NaN NaN NaN NaN
NaN NaN NaN NaN NaN NaN NaN
NaN NaN NaN NaN NaN NaN NaN
2012 11 2 3 4 5 6
2014 0 2 3 4 5 6
NaN NaN NaN NaN NaN NaN NaN
2013 0 31 3 4 5 6
2013 2 4 3 4 5 6
NaN NaN NaN NaN NaN NaN NaN
2013 1 2 2 59 5 6
2013 1 2 3 24 5 6
NaN NaN NaN NaN NaN NaN NaN
2013 1 2 3 3 59 6
2013 1 2 3 5 0 6
NaN NaN NaN NaN NaN NaN NaN
2013 1 2 3 4 4 999
2013 1 2 3 4 6 0
NaN NaN NaN NaN NaN NaN NaN
2012 1 2 3 4 5 6
2012 1 2 3 4 5 6
2012 1 2 3 4 5 6
2012 1 2 3 4 5 0
2012 1 2 3 4 0 0
2012 1 2 3 0 0 0
2012 1 2 0 0 0 0
2012 1 1 0 0 0 0
===*/

/* Date constructor tests */

print('date constructor tests');

function dateConstructorTest() {
    var d;

    function toUtcAndPrint(dt) {
        var utcDiff = utcDiffAtTime(dt);
        dt = new Date(dt.getTime() + utcDiff);  // Now UTC time
        print(dateToComponentString(dt));
    }

    // No arguments: current time
    d = new Date();
    print(d.getUTCFullYear() >= 2000 && d.getUTCFullYear() <= 2100);

    // One argument
    d = new Date(60*1000);  // 1970-01-01 00:01:00
    print(dateToComponentString(d));

    // One argument which doesn't parse
    d = new Date('foo');
    print(dateToComponentString(d));

    // One argument + an undefined argument counts as two arguments
    // Month is NaN -> time value is NaN -> Invalid Date
    d = new Date(2013, undefined);
    print(d);

    // Two or more arguments: year, month, etc
    // Note that these are interpreted as local time, so we compensate
    // the difference to ensure test result is independent of timezone

    d = new Date(2013, 3);
    toUtcAndPrint(d);

    d = new Date(2013, 1, 2, 3, 4, 5, 6);
    toUtcAndPrint(d);

    // try large (absolute) years
    d = new Date(-200000, 1, 2, 3, 4, 5, 6);
    toUtcAndPrint(d);

    d = new Date(200000, 1, 2, 3, 4, 5, 6);
    toUtcAndPrint(d);

    // special handling of year arguments 0...99 -> interpreted as 1900...1999
    d = new Date(-1, 1, 2, 3, 4, 5, 6);
    toUtcAndPrint(d);
    d = new Date(0, 1, 2, 3, 4, 5, 6);
    toUtcAndPrint(d);
    d = new Date(99, 1, 2, 3, 4, 5, 6);
    toUtcAndPrint(d);
    d = new Date(100, 1, 2, 3, 4, 5, 6);
    toUtcAndPrint(d);

    // invalid component values -> NaN -> Invalid Date;
    // in many cases out-of-range values are allowed and "normalized" by
    // MakeDate / MakeTime
    d = new Date(300000, 1, 2, 3, 4, 5, 6); toUtcAndPrint(d);
    d = new Date(-300000, 1, 2, 3, 4, 5, 6); toUtcAndPrint(d);
    d = new Date('foo', 1, 2, 3, 4, 5, 6); toUtcAndPrint(d);
    d = new Date(2013, -1, 2, 3, 4, 5, 6); toUtcAndPrint(d);    // out of range months affect year (see MakeDate),
    d = new Date(2013, 12, 2, 3, 4, 5, 6); toUtcAndPrint(d);    // so these two result in valid dates
    d = new Date(2013, 'foo', 2, 3, 4, 5, 6); toUtcAndPrint(d);
    d = new Date(2013, 1, 0, 3, 4, 5, 6); toUtcAndPrint(d);     // again, out of range days are allowed (MakeDate)
    d = new Date(2013, 1, 32, 3, 4, 5, 6); toUtcAndPrint(d);    // -""-
    d = new Date(2013, 1, 'foo', 3, 4, 5, 6); toUtcAndPrint(d);
    d = new Date(2013, 1, 2, 3, -1, 5, 6); toUtcAndPrint(d);    // allowed
    d = new Date(2013, 1, 2, 3, 24, 5, 6); toUtcAndPrint(d);    // -""-
    d = new Date(2013, 1, 2, 3, 'foo', 5, 6); toUtcAndPrint(d);
    d = new Date(2013, 1, 2, 3, 4, -1, 6); toUtcAndPrint(d);    // allowed
    d = new Date(2013, 1, 2, 3, 4, 60, 6); toUtcAndPrint(d);    // -""-
    d = new Date(2013, 1, 2, 3, 4, 'foo', 6); toUtcAndPrint(d);
    d = new Date(2013, 1, 2, 3, 4, 5, -1); toUtcAndPrint(d);    // allowed
    d = new Date(2013, 1, 2, 3, 4, 5, 1000); toUtcAndPrint(d);  // -""-
    d = new Date(2013, 1, 2, 3, 4, 5, 'foo'); toUtcAndPrint(d);

    // express a large timestamp with milliseconds only
    d = new Date(2012, 1, 2, 3, 4, 5, 6); toUtcAndPrint(d);           // this ...
    d = new Date(2012, 0, 0, 0, 0, 0, 2862245006); toUtcAndPrint(d);  // ... is equivalent to this

    // argument defaulting
    d = new Date(2012, 1, 2, 3, 4, 5, 6); toUtcAndPrint(d);
    d = new Date(2012, 1, 2, 3, 4, 5); toUtcAndPrint(d);
    d = new Date(2012, 1, 2, 3, 4); toUtcAndPrint(d);
    d = new Date(2012, 1, 2, 3); toUtcAndPrint(d);
    d = new Date(2012, 1, 2); toUtcAndPrint(d);
    d = new Date(2012, 1); toUtcAndPrint(d);
}

try {
    dateConstructorTest();
} catch (e) {
    print(e.name);
}
