/*
 *  Literal as a left-hand side expression (E5 Sections 11.2, 11.13,
 *  11.3.1, 11.3.2, 11.4.4, 11.4.5).
 */

/*===
ReferenceError
ReferenceError
ReferenceError
ReferenceError
===*/

try {
    1 = 2;
} catch(e) {
    print(e.name);
}

try {
    "foo" = 3;
} catch(e) {
    print(e.name);
}

try {
    /* Note: parenthesis required to interpret as an object literal, and not a block */
    ({ foo:4 }) = 5;
} catch(e) {
    print(e.name);
}

try {
    [6,7] = 8;
} catch(e) {
    print(e.name);
}

/* FIXME: inside function */
