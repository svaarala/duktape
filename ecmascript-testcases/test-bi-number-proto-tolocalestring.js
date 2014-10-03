/*---
{
    "custom": true
}
---*/

/* toLocaleString() behavior is not specified, except that the number
 * should be formatted "according to the conventions of the host
 * environment's current locale".  An implementation is allowed to do
 * the same thing as toString(), although this is not recommended.
 *
 * The current implementation uses toString() directly, so test for
 * this basic behavior.
 */

/*===
-Infinity
string -Infinity
-1000000000
string -1000000000
-123.4
string -123.4
-1
string -1
0
string 0
0
string 0
1
string 1
123.4
string 123.4
1000000000
string 1000000000
Infinity
string Infinity
undefined
string NaN
64
===*/

function basicTest() {
    function test(x) {
        var obj;
        var t;

        try {
            obj = new Number(x);
            t = obj.toString();
            print(typeof t, t);
        } catch (e) {
            print(e.name);
        }
    }

    var values = [ Number.NEGATIVE_INFINITY, -1e9, -123.4, -1, -0, +0, 1, 123.4, 1e9, Number.POSITIVE_INFINITY, Number.NAN ];
    var i;
    for (i = 0; i < values.length; i++) {
        print(values[i]);
        test(values[i]);
    }

    // radix argument is passed to toString now

    print(new Number(100).toLocaleString(16));
}

try {
    basicTest();
} catch (e) {
    print(e);
}
