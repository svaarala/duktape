/*
 *  RegExp toString() has strict semantics, although the output is not
 *  uniquely defined.  The specification only requires that the regexp
 *  can be compiled and will be semantically equivalent.
 *
 *  Hence, e.g. /\n[a-c]/i.toString() might result in:
 *
 *     1. /\n[a-c]/i
 *     2. /\n(?:a|b|c)/i
 *     3. /\u000a[A-C]/i
 *
 *  or one of many other alternatives.  Here, though, the expected string
 *  is written to match what we -want- to see.
 *
 *  Also note that output flag order is strictly specified.
 */

var r;

/*
 *  Test various regexp constructs and their escaping.  Pattern
 *  syntax is defined in E5 Section 15.10.1.
 */

/*===
/(?:)/
/^$\b\B(?=foo)(?!bar)/
/a*b*?c+d+?e?f??/
/a{3}b{3}?c{3,}d{3,}?e{3,10}f{3,10}?/
/a.\0\cD\xde\u1234\u00de\=/
/[abc-d][^abc-d]/
/fOO/i
===*/

try {
    r = new RegExp('');    /* empty regexp is a special case */
    print(r.toString());
} catch (e) {
    print(e.name);
}

try {
    r = /^$\b\B(?=foo)(?!bar)/;
    print(r.toString());
} catch (e) {
    print(e.name);
}

try {
    r = /a*b*?c+d+?e?f??/;
    print(r.toString());
} catch (e) {
    print(e.name);
}

try {
    r = /a{3}b{3}?c{3,}d{3,}?e{3,10}f{3,10}?/;
    print(r.toString());
} catch (e) {
    print(e.name);
}

try {
    /* '\cD' is a ControlLetter escape
     *.
     *  \= is an IdentityEscape, whose and should match '='.
     * Note that an IdentityEscape cannot be an IdentifierPart
     * (e.g. 'z').
     */
    r = /a.\0\cD\xde\u1234\u00de\=/;
    print(r.toString());
} catch (e) {
    print(e.name);
}

try {
    /* XXX: much to expand here */
    r = /[abc-d][^abc-d]/;
    print(r.toString());
} catch (e) {
    print(e.name);
}

try {
    /* case normalization would be allowed in pattern here */
    r = /fOO/i;
    print(r.toString());
} catch (e) {
    print(e.name);
}

/*
 *  Flags
 */

/*===
/foo/g
/foo/i
/foo/m
/foo/gim
===*/

try {
    r = /foo/g;
    print(r.toString());

    r = /foo/i;
    print(r.toString());

    r = /foo/m;
    print(r.toString());

    /* Note: input flag order differs from -required- output (E5 Section 15.10.6.4) */
    r = /foo/mig;
    print(r.toString());
} catch (e) {
    print(e.name);
}
