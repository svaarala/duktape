/*
 *  Duktape has an internal digit limit (9 digits) for regexp quantifier
 *  min/max counts.
 */

/*---
{
    "custom": true
}
---*/

/*===
["xxx"]
null
===*/

// 8 digits
try {
    print(eval("JSON.stringify(/x{3,99999999}/.exec('xxx'))"));
} catch (e) {
    print(e);
}
try {
    print(eval("JSON.stringify(/x{88888888,99999999}/.exec('xxx'))"));
} catch (e) {
    print(e);
}

/*===
["xxx"]
null
===*/

// 9 digits, still accepted
try {
    print(eval("JSON.stringify(/x{3,999999999}/.exec('xxx'))"));
} catch (e) {
    print(e);
}
try {
    print(eval("JSON.stringify(/x{333333333,999999999}/.exec('xxx'))"));
} catch (e) {
    print(e);
}

/*===
null
null
===*/

// 10 digits: SyntaxError without non-standard literal curly braces
// (DUK_USE_ES6_REGEXP_SYNTAX), treated as a literal with non-standard
// curly braces.
try {
    print(eval("JSON.stringify(/x{3,9999999999}/.exec('xxx'))"));
} catch (e) {
    print(e);
}
try {
    print(eval("JSON.stringify(/x{3333333333,9999999999}/.exec('xxx'))"));
} catch (e) {
    print(e);
}
