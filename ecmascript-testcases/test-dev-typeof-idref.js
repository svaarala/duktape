/*
 *  The 'typeof' operator requires that an 'undefined' value is returned
 *  if 'typeof' is applied to an unresolvable reference (E5 Section 11.4.3).
 *  Usually unresolvable references cause a ReferenceError.
 *
 *  This requires special handling in the compiler, which emits, in the worst
 *  case, an TYPEOFID opcode for handling this run-time.  Any compile-time
 *  resolvable references (concretely: register bound identifiers which are
 *  always non-deletable) don't require this special handling.
 *
 */

/*===
undefined
undefined
===*/

try {
    print(typeof foo);  // unresolvable -> undefined; requires TYPEOFID
} catch (e) {
    print(e.name);
}

try {
    print(typeof ((foo)));  // parens must have no effect
} catch (e) {
    print(e.name);
}

/*===
number
===*/

function func(x) {
    // register-bound variables are always resolvable
    return typeof x;
}

print(func(3));
