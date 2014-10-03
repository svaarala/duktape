/*===
123
123
123
123
123
-123
-123
===*/

/* The internal time value of an Ecmascript Date has no fractions.
 * For instance, TimeClip() applies ToInteger() for all finite
 * values.
 *
 * Check that this is the case.  Also check that rounding happens
 * correctly; ToInteger() rounds towards zero.
 */

try {
    print(new Date(2012, 0, 1, 11, 22, 33, 123).getTime() % 1000);
    print(new Date(2012, 0, 1, 11, 22, 33, 123.1).getTime() % 1000);
    print(new Date(2012, 0, 1, 11, 22, 33, 123.9).getTime() % 1000);

    print(new Date(123.1).getTime());
    print(new Date(123.9).getTime());
    print(new Date(-123.1).getTime());
    print(new Date(-123.9).getTime());
} catch (e) {
    print(e.name);
}
