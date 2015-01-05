/*
 *  Some invalid character class cases.
 */

/*===
SyntaxError
===*/

/* Should cause a SyntaxError, see E5 Section 15.10.2.15. */
try {
    /* '\d' contains [0-9], while 'z' is a single character.
     * CharacterRange() (E5 Section 15.10.2.15) should reject
     * this with SyntaxError because both endpoints must be
     * single characters.
     */

    /* Note: Rhino and V8 both accept this. */

    /* FIXME: there is a bug which causes us to accept this as well */

    eval('t = /[\\d-z]/;');
    print("no error");
} catch(e) {
    print(e.name);
}

/*===
SyntaxError
===*/

/* Should cause a SyntaxError, see E5 Section 15.10.2.15. */
try {
    eval('t = /[z-x]/;');
} catch(e) {
    print(e.name);
}
