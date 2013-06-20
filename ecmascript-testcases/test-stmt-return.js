/*
 *  Return statement (E5 Section 12.9).
 */

/* FIXME: more tests */

/*===
SyntaxError
===*/

/* SyntaxError outside of a function body, E5 Section 12.9 */

try {
    eval("return 1");
} catch (e) {
    print(e.name);
}

