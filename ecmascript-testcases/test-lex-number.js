/*
 *  Parsing of numeric literals (E5 Sections 7.8.3, B.1.1).
 */

/*---
{
    "skip": true
}
---*/

/*
 *  Optional octal syntax, E5 Section B.1.1.
 */

/*===
1036956
63
SyntaxError
SyntaxError
===*/

try {
    print(eval('03751234'));
} catch (e) {
    print(e.name);
}

try {
    print(eval('077'));
} catch (e) {
    print(e.name);
}

// These are not valid octal, but both Rhino and V8 parse them as decimal
// (088 -> 88, 099 -> 099, but 077 -> 63).  At the moment Duktape throws a
// SyntaxError for these.

try {
    print(eval('088'));
} catch (e) {
    print(e.name);
}

try {
    print(eval('099'));
} catch (e) {
    print(e.name);
}

/*===
1
true
===*/

/* Fraction period not necessarily followed by decimals. */

try {
    print(1.);
    print(1. === 1);
} catch (e) {
    print(e.name);
}
