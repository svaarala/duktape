/*===
Invalid Date
NaN
[object Date]
===*/

/* E5.1 Section 15.9.5: Date.prototype is a 'Date' instance (internal class
 * is "Date", and time value is NaN.
 */

try {
    print(Date.prototype.toString());
    print(Date.prototype.getTime());
    print(Object.prototype.toString.call(Date.prototype));
} catch (e) {
    print(e.name);
}
