/*===
ReferenceError
===*/

/* Valid LeftHandSideExpression which is invalid for assignment
 * is a run-time ReferenceError, not a SyntaxError.  E5 allows
 * a LHS Reference to be returned from a function, although it
 * provides no standard way of doing so.
 */

try {
    eval("f() = 1");
} catch (e) {
    print(e.name);
}
