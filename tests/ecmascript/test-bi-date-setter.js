/*===
1970-01-01T00:00:00.000Z
1981-06-10T12:34:56.123Z
RangeError
2000-01-01T00:00:00.000Z
RangeError
true false
RangeError
true false
1970-01-01T00:00:00.000Z
true true
1970-01-01T00:00:00.000Z
true true
===*/

function toISO(d) {
    try {
        return d.toISOString();  // can throw RangeError
    } catch (e) {
        return e.name;
    }
}

function testSetters() {
    var d;

    d = new Date(0);  // utc
    print(toISO(d));

    // test string coercion
    d.setUTCFullYear('1981');
    d.setUTCMonth('5');
    d.setUTCDate('10');
    d.setUTCHours('12');
    d.setUTCMinutes('34');
    d.setUTCSeconds('56');
    d.setUTCMilliseconds('123');
    print(toISO(d));

    // year setters restore a NaN time value to a +0 before setting the
    // year (unlike all other setters)
    d = new Date('foo');
    print(toISO(d));
    d.setUTCFullYear(2000);
    print(toISO(d));  // utc time known exactly, can check exact string

    d = new Date('foo');
    print(toISO(d));
    d.setFullYear(2000);
    print(toISO(d) !== 'RangeError', isNaN(d.getTime()));

    d = new Date('foo');
    print(toISO(d));
    d.setYear(2000);
    print(toISO(d) !== 'RangeError', isNaN(d.getTime()));

    // Setting an infinity results in a NaN time value
    d = new Date(0);
    print(toISO(d));
    d.setYear(1/0);
    print(toISO(d) === 'RangeError', isNaN(d.getTime()));
    d = new Date(0);
    print(toISO(d));
    d.setYear(1/0);
    print(toISO(d) === 'RangeError', isNaN(d.getTime()));
}

try {
    testSetters();
} catch (e) {
    print(e.name, e);
}
