var r;
var src;
var txt;
var i;

/*===
SyntaxError
===*/

/*
 *  '\z' is not a valid escape: it is close to being an IdentityEscape,
 *  but an IdentityEscape can only be:
 *
 *     SourceCharacter but not IdentifierPart
 *     <ZWJ>
 *     <ZWNJ>
 *
 *  Note: Rhino and V8 will accept this.
 */

try {
    // wrap in eval because it's a SyntaxError
    r = eval("/\\z/");
    print(r.toString());
} catch (e) {
    print(e.name);
}

/*===
47
8205
47
47
92
8205
47
===*/

/* ZWJ
 *
 * http://en.wikipedia.org/wiki/Zero-width_joiner
 *
 * Avoid printing the character explicitly.
 */

try {
    src = '\u200d';
    r = new RegExp(src);
    txt = r.toString();
    for (i = 0; i < txt.length; i++) {
        print(txt.charCodeAt(i));
    }
} catch (e) {
    print(e.name);
}

try {
    // ZWJ in an identity escape
    src = '\\' + '\u200d';
    r = new RegExp(src);
    txt = r.toString();
    for (i = 0; i < txt.length; i++) {
        print(txt.charCodeAt(i));
    }
} catch (e) {
    print(e.name);
}

/*===
47
8204
47
47
92
8204
47
===*/

/* ZWNJ
 *
 * http://en.wikipedia.org/wiki/Zero-width_non-joiner
 */

try {
    src = '\u200c';
    r = new RegExp(src);
    txt = r.toString();
    for (i = 0; i < txt.length; i++) {
        print(txt.charCodeAt(i));
    }
} catch (e) {
    print(e.name);
}

try {
    // ZWNJ in an identity escape
    src = '\\' + '\u200c';
    r = new RegExp(src);
    txt = r.toString();
    for (i = 0; i < txt.length; i++) {
        print(txt.charCodeAt(i));
    }
} catch (e) {
    print(e.name);
}
