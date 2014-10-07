/*
 *  Backreferences in the pattern part are covered, rather confusingly:
 *
 *    - E5 Section 15.10.2.9: AtomEscape production, which first
 *      evaluates the DecimalEscape production and looks at the result.
 *
 *    - E5 Section 15.10.2.11: DecimalEscape production; returns either
 *      a NUL character or a non-zero number.
 *
 *  This is rather confusing in that since DecimalEscape cannot evaluate
 *  to a zero number (\0 generates NUL character matcher), the check for
 *  zero in E5 Section 15.10.2.9 step 4 seems unnecessary.  Also note
 *  that (quoted from E5 Section 15.10.2.11).
 *
 *    "NOTE: If \ is followed by a decimal number n whose first digit
 *     is not 0, then the escape sequence is considered to be a
 *     backreference. It is an error if n is greater than the total
 *     number of left capturing parentheses in the entire regular
 *     expression. \0 represents the <NUL> character and cannot be
 *     followed by a decimal digit."
 *
 *  The evaluation algorithm in E5 Section 15.10.2.9 contains the actual
 *  behavior; more specifically step 4 throws a SyntaxError if "n=0 or
 *  n>NCapturingParens".
 *
 *  This means that:
 *
 *    - '\0' is a character match for U+0000
 *    - '\00' is apparently a SyntaxError
 *    - '\1' is a backreference, and a SyntaxError if there are no captures
 *
 *  If \n is in [1,NCapturingParens], but has not captured anything (perhaps
 *  it was an alternative in a disjunction that was not matched, or the capture
 *  only happens in the future), then the backreference should always SUCCEED!
 *  See E5 Section 15.10.2.9, step 5.3.
 */

var t;

/*===
null
102 111 111 0 4
SyntaxError
SyntaxError
SyntaxError
foo foo
fooquux undefined
===*/

/* Note: double quote backslashes carefully below. */

/*
 *  XXX: neither Rhino nor V8 will throw SyntaxError from these cases
 *  (they'll just return a null match).
 */

try {
    /* no match -> null */
    eval("t = /foo\\0/.exec('foobar'); print(t);");
} catch (e) {
    print(e.name);
}

try {
    /* matches U+0000 -> t[0] = 'foo\u0000', avoid printing it directly */
    eval("t = /foo\\0/.exec('foo\\u0000'); print(t[0].charCodeAt(0), t[0].charCodeAt(1), t[0].charCodeAt(2), t[0].charCodeAt(3), t[0].length);");
} catch (e) {
    print(e.name);
}

try {
    /* '\0' must not be followed by decimal digits -> SyntaxError */
    eval("t = /foo\\00/.exec('foobar'); print(t);");
} catch (e) {
    print(e.name);
}

try {
    /* same as above */
    eval("t = /foo\\01/.exec('foobar'); print(t);");
} catch (e) {
    print(e.name);
}

try {
    /* not allowed, > NCapturingParen (=0) -> SyntaxError
     *
     * Note: Rhino will throw an *uncatchable* NullPointerException here!
     */

    eval("t = /foo\\1/.exec('foobar'); print(t);");
} catch (e) {
    print(e.name);
}

try {
    /* allowed, backreference is ignored, only 'foo' is matched */
    eval("t = /\\1(foo)/.exec('foobar'); print(t[0], t[1]);");
} catch (e) {
    print(e.name);
}

try {
    /* (?:foo|(bar)) will match 'foo', and capture \1 will be undefined.
     * The \1 will then succeed without matching any input.
     * Finally, 'quux' is matched.
     */
    eval("t = /(?:foo|(bar))\\1quux/.exec('fooquux'); print(t[0], t[1]);");
} catch (e) {
    print(e.name);
}
