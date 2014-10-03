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
===*/
try {
    print(03751234);
} catch (e) {
    print(e.name);
}

/* FIXME: test 088, 099 */

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
