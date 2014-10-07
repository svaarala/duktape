/*===
SyntaxError
1
===*/

/* Escaped characters allow keywords to be used e.g. as variable names;
 * e.g. "if" with first "i" escaped is no longer recognized as a keyword.
 *
 * This matches behavior in V8 and Rhino, but is not necessarily compliant.
 * See E5 Section 7.6:
 *
 *    "All interpretations of identifiers within this specification are
 *     based upon their actual characters regardless of whether or not an
 *     escape sequence was used to contribute any particular characters."
 */

try {
    eval("if = 1;");
} catch (e) {
    print(e.name);
}

try {
    eval("\\u0069f = 1; print(\\u0069f);");  // double escape on purpose
} catch (e) {
    print(e.name);
}
