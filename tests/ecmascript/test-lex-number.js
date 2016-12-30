/*
 *  Parsing of number literals (E5 Sections 7.8.3, B.1.1).
 */

/*
 *  Optional octal syntax, E5 Section B.1.1.
 */

/*===
1036956
63
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

/*===
false
SyntaxError
===*/

/* Specific requirement for parsing in ES2015 Section 11.8.3. */

try {
    print(eval('3 in {}'));
} catch (e) {
    print(e.name);
}
try {
    print(eval('3in {}'));
} catch (e) {
    print(e.name);
}
