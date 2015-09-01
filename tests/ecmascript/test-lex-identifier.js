/*
 *  Parsing of identifier names (E5 Section 7.6).
 */

/*
 *  Some specific identifier names
 */

/*===
10
20
===*/

try {
    eval('$=10; print($);');
} catch (e) {
    print(e.name);
}

try {
    eval('_=20; print(_);');
} catch (e) {
    print(e.name);
}


/*
 *  Escapes in IdentifierStart and IdentifierPart
 */

/*===
SyntaxError
SyntaxError
val
3
SyntaxError
===*/

/* These test were added because Rhino fails some, and some early code
 * failed here too.  It's easy to forget a check if the char is escaped,
 * or to apply an IdentifierPart check for an escaped IdentifierStart
 * (IdentifierPart is wider).
 */

/* Rhino fails */
try {
    eval('\\u0001x = "val";');   /* U+0001 not allowed in IdentifierStart or IdentifierPart */
    print('1');
} catch (e) {
    print(e.name);
}

/* Rhino fails */
try {
    eval('\\u0030x = "val";');   /* identifier '0x', not allowed */
    print('2');
} catch (e) {
    print(e.name);
}

try {
    eval('x\\u0030 = "val";');   /* identifier 'x0', allowed */
    print(x0);
    print('3');
} catch (e) {
    print(e.name);
}

/* There is no hex escape for identifier names. */
try {
    eval('x\\x30 = "val";');   /* identifier 'x0' */
    print('4');
} catch (e) {
    print(e.name);
}

/* XXX: add more cases */
