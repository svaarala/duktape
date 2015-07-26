/*===
66
66
===*/

/* U+2028 and U+2029 were considered incorrectly as both line terminators
 * and white space characters (white space included category Z, not just
 * category Zs).  This caused odd behavior in code below: the U+2028 and
 * U+2029 terminate identifier parsing but are then skipped without producing
 * a lineterm token.  This breaks automatic semicolon insertion.
 */

var bar;

try {
    bar = -1;
    eval("var foo\u2028bar = 66;");
    print(bar);
} catch (e) {
    print(e);
}

try {
    bar = -1;
    eval("var foo\u2028bar = 66;");
    print(bar);
} catch (e) {
    print(e);
}
