/*===
8640000000000000
NaN
-8640000000000000
NaN
NaN
NaN
NaN
0
0
10
10
0
0
-10
-10
Infinity
implementation dependent -Infinity
===*/

/* Test TimeClip() limits when creating timestamps from time values directly.
 * E5.1 Section 15.9.3.2 (new Date(value)): even in this case the value given
 * will go through TimeClip().
 *
 * E5.1 Section 15.9.1.14.
 */

function timeClipTest() {
    var d;

    // upper range
    d = new Date(8.64000000e15); print(d.getTime());
    d = new Date(8.64000000e15 + 1); print(d.getTime());

    // lower range
    d = new Date(-8.64000000e15); print(d.getTime());
    d = new Date(-8.64000000e15 - 1); print(d.getTime());

    // non-finite numbers
    d = new Date(Number.NaN); print(d.getTime());
    d = new Date(Number.POSITIVE_INFINITY); print(d.getTime());
    d = new Date(Number.NEGATIVE_INFINITY); print(d.getTime());

    // TimeClip() removes any fractional milliseconds using ToInteger(), which
    // will always round towards zero.
    d = new Date(0.1); print(d.getTime());
    d = new Date(0.9); print(d.getTime());
    d = new Date(10.1); print(d.getTime());
    d = new Date(10.9); print(d.getTime());
    d = new Date(-0.1); print(d.getTime());
    d = new Date(-0.9); print(d.getTime());
    d = new Date(-10.1); print(d.getTime());
    d = new Date(-10.9); print(d.getTime());

    // TimeClip() may (but is not required) to convert a negative zero
    // to a positive one.  Our current implementation will preserve the
    // sign so test for that behavior (V8 has the same behavior, Rhino
    // does not).

    // FIXME: custom
    d = new Date(+0); print(1 / d.getTime());
    d = new Date(-0); print('implementation dependent', 1 / d.getTime());
}

try {
    timeClipTest();
} catch (e) {
    print(e.name);
}
