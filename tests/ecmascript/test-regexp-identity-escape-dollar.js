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
 *
 * This is a bit problematic for practical regular expressions: since a '$'
 * has a special meaning, a literal dollar sign needs to be expressed as a
 * numeric character escape which is quite awkward.
 *
 * Real world code seems to contain these '\$' escapes, so the current
 * expectation is to allow them, both inside and outside of character
 * classes.
 */

/*===
object
object $
object
object $xx$
object
object $
object
object $xx$
object
object $xx$
===*/

function invalidDollarEscape() {
    var re, m;

    re = eval("/\\$/");
    print(typeof re);
    m = re.exec('foo$bar');
    print(typeof m, m[0]);

    re = eval("/[\\$x]+/");
    print(typeof re);
    m = re.exec('$xx$');
    print(typeof m, m[0]);
}

function validDollarEscape() {
    var re, m;

    re = eval("/\\u0024/");
    print(typeof re);
    m = re.exec('foo$bar');
    print(typeof m, m[0]);

    re = eval("/[\\u0024x]+/");
    print(typeof re);
    m = re.exec('$xx$');
    print(typeof m, m[0]);

    // a literal dollar is also allowed inside character classes
    re = eval("/[$x]+/");
    print(typeof re);
    m = re.exec('$xx$');
    print(typeof m, m[0]);
}

try {
    invalidDollarEscape();
} catch (e) {
    print(e.name);
}

try {
    validDollarEscape();
} catch (e) {
    print(e.name);
}
