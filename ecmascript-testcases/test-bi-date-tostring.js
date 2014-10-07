/*---
{
    "custom": true
}
---*/

/*===
toUTCString="1970-01-01 00:00:00.000Z" toGMTString="1970-01-01 00:00:00.000Z" toISOString="1970-01-01T00:00:00.000Z" toJSON="1970-01-01T00:00:00.000Z"
toUTCString="-200000-02-02 03:04:05.006Z" toGMTString="-200000-02-02 03:04:05.006Z" toISOString="-200000-02-02T03:04:05.006Z" toJSON="-200000-02-02T03:04:05.006Z"
toUTCString="+200000-02-02 03:04:05.006Z" toGMTString="+200000-02-02 03:04:05.006Z" toISOString="+200000-02-02T03:04:05.006Z" toJSON="+200000-02-02T03:04:05.006Z"
toUTCString="Invalid Date" toGMTString="Invalid Date" toISOString="RangeError" toJSON="null"
===*/

/* Many conversion functions are implementation specific.  Most are also
 * timezone and/or locale dependent, which makes these functions difficult
 * to test without the ability to force locale / timezone.
 *
 * Here we test our time representation, so the test is necessarily a
 * custom one.
 *
 * Tested functions: toString(), toDateString(), toTimeString(),
 * toLocaleString(), toLocaleDateString(), toLocaleTimeString(),
 * toUTCString(), toISOString(), toJSON(key), and valueOf().
 */

function printConversions(dt) {
    var tmp = [];
    var t;

    function f(n,v) {
        if (typeof v !== 'string') {
            throw new Error('invalid type: ' + v);
        }
        tmp.push(n + '="' + String(v) + '"');
    }

    // XXX: how to test these, as even the custom output depends on
    // build options and current locale?
    /*
    f('toString', dt.toString());
    f('toDateString', dt.toDateString());
    f('toTimeString', dt.toTimeString());
    f('toLocaleString', dt.toLocaleString());
    f('toLocaleDateString', dt.toLocaleDateString());
    f('toLocaleTimeString', dt.toLocaleTimeString());
    */

    f('toUTCString', dt.toUTCString());
    f('toGMTString', dt.toUTCString());
    try {
        f('toISOString', dt.toISOString());
    } catch (e) {
        // RangeError is possible
        f('toISOString', e.name);
    }
    t = dt.toJSON();
    if (t === null) {
        // null is possible
        t = 'null';
    }
    f('toJSON', t);

    print(tmp.join(' '));
}

function datePrototypeStringConversion() {
    var pc = printConversions;

    pc(new Date(0));  // single argument is 'utc time'

    pc(new Date(Date.UTC(-200000, 1, 2, 3, 4, 5, 6)));
    pc(new Date(Date.UTC(200000, 1, 2, 3, 4, 5, 6)));
    pc(new Date(Date.UTC(1800000000000e3)));
}

try {
    datePrototypeStringConversion();
} catch (e) {
    print(e.name, e);
}

/*===
Invalid Date
RangeError
===*/

try {
    // toISOString() is required to throw a RangeError rather than return
    // an 'Invalid Date' if time value is invalid.  Other converters don't
    // have this property.
    print(new Date(NaN).toUTCString());
    print(new Date(NaN).toISOString());
} catch (e) {
    print(e.name);
}
