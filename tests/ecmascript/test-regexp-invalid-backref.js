/*
 *  Invalid backreferences.
 */

/*===
SyntaxError
SyntaxError
===*/

/* E5 Section 15.10.2.9: n > NCapturingParens */
try {
    eval("/\\1/.exec('foo');");
    print("no exception");
} catch (e) {
    print(e.name);
}

/* Same error but never executed by regexp engine because an
 * earlier match succeeds.  However, because E5 Section 15.10.2.9
 * deals with 'compilation' of an internal [[Match]] algorithm at
 * regexp compilation time, a SyntaxError should happen when the
 * RegExp is created.
 *
 * Note that a RegExp literal such as /\1/ should cause a
 * compile-time syntax error when compiling the global program,
 * so eval is used.
 */
try {
    eval("/foo|\\1/.exec('foo');");
    print("no exception");
} catch (e) {
    print(e.name);
}
