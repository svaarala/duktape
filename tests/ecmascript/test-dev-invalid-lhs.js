/*
 *  LeftHandSideExpression allows a much wider syntactical set of
 *  lvalues than we actually support.  Valid LHS values which we
 *  can't write to must not cause SyntaxError but a ReferenceError.
 *
 *  The side effects of the LHS and the RHS (in that order) must
 *  happen before the ReferenceError.
 */

/* XXX: is this correct?  Both Rhino and V8 will just throw a
 * ReferenceError.
 */

/*===
lhs
rhs
ReferenceError
===*/

try {
    eval("print('lhs') = print('rhs');");
} catch(e) {
    print(e.name);
}
