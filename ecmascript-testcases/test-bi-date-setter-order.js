/*===
setter order test
NaN
-8639980560000000
===*/

/* Order of using setters matters -- if any intermediate timestamp goes out
 * of E5 range, the time value will become NaN and won't recover by further
 * setter calls, even if they would bring the timestamp back to valid range.
 */

print('setter order test');

function setterOrderTest() {
    var d;

    d = new Date(0);
    d.setUTCFullYear(-271821);  // year=-271821, month=0 is not in valid range -> timeval NaN
    d.setUTCMonth(11);          // year=-271821, month=11 *is* in valid range, but we'll remain NaN here
    d.setUTCDate(1);
    d.setUTCHours(0);
    d.setUTCMinutes(0);
    d.setUTCSeconds(0);
    d.setUTCMilliseconds(0);

    // NaN is required here by the specification.
    print(d.getTime());

    // This order has no issues
    d = new Date(0);
    d.setUTCMilliseconds(0);
    d.setUTCSeconds(0);
    d.setUTCMinutes(0);
    d.setUTCHours(0);
    d.setUTCDate(1);
    d.setUTCMonth(11);
    d.setUTCFullYear(-271821);
    print(d.getTime());
}

try {
    setterOrderTest();
} catch (e) {
    print(e.name);
}
