/*---
{
    "custom": true
}
---*/

/*===
2012-01-01T00:00:00.000Z
2012-01-01T00:00:00.000Z
0322-01-01T00:00:00.000Z
2012-01-01T00:00:00.000Z
2012-01-01T00:00:00.000Z
0322-01-01T00:00:00.000Z
-002012-01-01T00:00:00.000Z
-002012-01-01T00:00:00.000Z
-000322-01-01T00:00:00.000Z
0001-02-03T05:07:06.700Z
0001-02-03T05:07:06.700Z
0001-02-03T05:07:06.700Z
RangeError
2012-01-02T03:04:05.100Z
2012-01-02T03:04:05.120Z
2012-01-02T03:04:05.123Z
2012-01-02T03:04:05.123Z
2012-01-02T03:04:05.123Z
2012-01-02T12:00:00.000Z
2012-01-02T02:04:05.006Z
2012-01-02T04:04:05.006Z
2012-01-02T02:02:05.006Z
2020-03-02T02:02:05.006Z
2014-09-25T02:02:05.006Z
2012-01-06T02:02:05.006Z
2012-03-11T12:37:05.006Z
2012-01-02T02:03:39.006Z
2011-11-21T12:02:05.006Z
2011-12-26T03:25:05.006Z
2012-01-02T01:03:05.006Z
2012-01-02T05:05:05.006Z
2012-01-02T03:04:05.006Z
===*/

/* The ISO 8601 subset parser is very lenient, so check for that
 * leniency here.
 */

function test(str) {
    try {
        print(new Date(str).toISOString());
    } catch (e) {
        print(e.name);
    }
}

try {
    // year can have more (or less) than 4 digits even without +/-
    test('002012');
    test('00002012');
    test('322');
    test('+002012');
    test('+00002012');
    test('+322');
    test('-002012');
    test('-00002012');
    test('-322');

    // same applies to other components
    test('1-2-3T4:5:6.7-1:2');
    test('0001-0002-0003T0004:0005:0006.700000-000001:000002');

    // >9 digits in a component (other than millisecond) is not
    // accepted (does not fit into a signed 32-bit int)
    test('1-2-3T4:000000005:6.7-1:2');
    test('1-2-3T4:0000000005:6.7-1:2');

    // milliseconds can contain less than 3 and an unlimited
    // number of digits (anything beyond 3 is ignored)
    test('2012-01-02T03:04:05.1Z');
    test('2012-01-02T03:04:05.12');
    test('2012-01-02T03:04:05.123');
    test('2012-01-02T03:04:05.123456');
    test('2012-01-02T03:04:05.123456909090909090909090909090909090909090909090909090909090');

    // time part can contain just the hour part
    test('2012-01-02T12Z');

    // timezone offset part can contain just the hour part
    test('2012-01-02T03:04:05.006+01');
    test('2012-01-02T03:04:05.006-01');

    // any component (except milliseconds) can be unnormalized and
    // will be normalized correctly
    test('2012-01-02T03:04:05.006+01:02');
    test('2012-99-02T03:04:05.006+01:02');
    test('2012-01-999T03:04:05.006+01:02');
    test('2012-01-02T99:04:05.006+01:02');
    test('2012-01-02T03:99999:05.006+01:02');  // 99999 min = 69 days, 10 hours, 39 mins
    test('2012-01-02T03:04:99.006+01:02');
    test('2012-01-02T03:04:05.006+999:02');
    test('2012-01-02T03:04:05.006+01:9999');

    // positive timezone offset is subtracted before UTC conversion,
    // negative is added
    test('2012-01-02T03:04:05.006+02:01');
    test('2012-01-02T03:04:05.006-02:01');

    // date/time separator can be a space
    test('2012-01-02 03:04:05.006');
} catch (e) {
    print(e.name);
}

/*===
===*/

/* XXX: other custom parsing tests, like locale specific ones? */
