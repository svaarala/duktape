/*
 *  'void' operator (E5 Section 11.4.2).
 */

/*===
456
undefined
===*/

try {
    print(eval("123; 456;"));
} catch (e) {
    print(e.name);
}

try {
    /* latter expression generates an 'undefined' implicit return value */
    print(eval("123; void 456;"));
} catch (e) {
    print(e.name);
}
