/*===
SyntaxError
===*/

/* E5.1 Section 15.5.4.11 (String.prototype.replace) contains the following
 * example:
 *
 *   For example, "$1,$2".replace(/(\$(\d))/g, "$$1-$1$2") returns
 *   "$1-$11,$1-$22".
 *
 * The problem here is that the regexp contains the escape "\$", which is
 * not a valid IdentityEscape; from the RegExp syntax (E5.1 Section 15.10.1):
 *
 *   IdentityEscape ::
 *       SourceCharacter but not IdentifierPart
 *       <ZWJ>
 *       <ZWNJ>
 *
 * Because '$' is in IdentifierPart, it cannot be used as a valid identity
 * escape.  The E5.1 replace() example is thus incorrect (the backslash needs
 * to be omitted.
 */

try {
    var re = eval("/\\$/");
    print(typeof re);
    var m = re.exec('foo$bar');
    print(typeof m, m[0]);
} catch (e) {
    print(e.name);
}

