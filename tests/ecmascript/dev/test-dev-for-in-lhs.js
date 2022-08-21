function lhs() {
    print('lhs');
}

function getenum1() {
    print('getenum1');
    return {};
}

function getenum2() {
    print('getenum2');
    return { 'foo': 'bar' };
}

function getint() {
    print('getint');
    return 10;
}

/*===
getenum1
done
getenum2
lhs
ReferenceError
===*/

/* The left-hand-side of a for-in may be a function call (and some other
 * expressions allowed by LeftHandSideExpression).  This is NOT a SyntaxError
 * and will only produce a ReferenceError when the actual assignment is
 * attempted.
 *
 * Note that at least Rhino and V8 won't call the lhs() function before
 * throwing a ReferenceError.  This seems technically incorrect: E5 Section
 * 12.6.4 steps 6.b and 6.c seem to indicate that the LHS should be evaluated
 * first, and only the actual attempt to PutValue() to an invalid reference
 * should throw a ReferenceError.  The specification does allow leeway in
 * whether or not the LHS is evaluated repeatedly though.
 *
 * In any case, the current implementation will evaluate the lhs() before
 * throwing a ReferenceError, so the test case now tests for that behavior.
 */

try {
    /* Here the enumerator will be empty, so no ReferenceError should happen.
     * The lhs() expression is not evaluated at all.
     */

    for (lhs() in getenum1()) {
        print('loop');
    }
    print('done');
} catch (e) {
    print(e.name);
}

try {
    /* Here empty() is called, but ReferenceError occurs on first assignment. */

    for (lhs() in getenum2()) {
        print('loop');
    }
    print('done');
} catch (e) {
    print(e.name);
}

/*===
getint
getenum2
foo
===*/

/* The variable initializer of a for-in statement may have a value.
 * That value is computed before the enumeration target is evaluated.
 */

try {
    for (var i = getint() in getenum2()) {
        print(i);
    }
} catch (e) {
    print(e.name);
}
