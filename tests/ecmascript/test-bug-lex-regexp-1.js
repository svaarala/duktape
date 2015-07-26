/*===
62
===*/

/* This parsed incorrectly at one point, the slash was parsed as
 * part of a RegExp leading to SyntaxError.
 */

var obj, val;

try {
    eval("obj = { foo: 124 };");
    eval("val = obj['foo']/2;");
    eval("print(val)");
} catch (e) {
    print(e.name);
}

/*===
50
===*/

/* Similar test for parens */

try {
    print(eval("(50+50)/2"));
} catch (e) {
    print(e.name);
}

/*===
NaN
===*/

/* Similar test for braces (this doesn't evaluate to anything useful
 * but is not a SyntaxError.
 */

try {
    print(eval("({foo:1}/2)"));
} catch (e) {
    print(e.name);
}
