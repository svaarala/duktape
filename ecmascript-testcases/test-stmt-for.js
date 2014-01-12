/*
 *  For statement (E5 Section 12.6.3).
 */

/* FIXME: other tests */

/*===
SyntaxError
OK
===*/

try {
    /* SyntaxError, because the first part of the three-part for
     * must not contain a top level 'in'.
     */
    eval("x={}; for ('foo' in x; false; false) {};");
    print("never here");
} catch (e) {
    print(e.name);
}

try {
    /* No SyntaxError because an 'in' may appear inside parenthesis */
    eval("x={}; for (('foo' in x); false; false) {};");
    print("OK");
} catch (e) {
    print(e.name);
}

/*FIXME:break*/
/*FIXME:continue*/

