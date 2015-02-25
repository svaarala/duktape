/*
 *  Assignment eval order corner case
 *
 *  The RHS of an assignment may change the base variable of the LHS.  The
 *  write should go to the "original LHS".
 *
 *  In concrete terms, if the LHS is compiled into a property expression with
 *  a bound variable reference (register) as a base, and the variable is
 *  updated by the RHS, the write will go to the updated variable value.
 */

/*===
2
undefined
123
undefined
3
foo
bar
quux
true
===*/

function test() {
    var a = [];
    var b = [ 'foo', 'bar', 'quux' ];
    var orig_a = a;

    /* Because LHS is evaluated first, the write goes to the original 'a',
     * not 'a' after the assignment 'a = b'.
     */

    a[1] = ((a = b), 123);

    print(orig_a.length);
    print(orig_a[0]);
    print(orig_a[1]);
    print(orig_a[2]);

    print(b.length);
    print(b[0]);
    print(b[1]);
    print(b[2]);

    print(a == b);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
