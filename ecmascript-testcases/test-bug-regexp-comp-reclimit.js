/*===
compiled ok
===*/

/* The regexp compiler had a bug: recursion depth was incremented but never
 * decremented.  Hence a sequence of disjunctions, while having a low maximum
 * recursion depth, will trigger the recursion limit.
 */
try {
    var t, i;
    var r;

    t = '^';
    for (i = 0; i < 100; i++) {
        t = t + '(?:a|b)';
    }

    r = new RegExp(t);
    print('compiled ok');
} catch (e) {
    print(e.name);
}
