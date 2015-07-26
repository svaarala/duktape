/*
 *  Numeric property access.
 */

/*===
SyntaxError
===*/

try {
    /* Note: "x.5" should apparently be parsed as the two tokens
     * "x" (identifier) and ".5" (decimal number)
     */
    eval("x={5:6}; print(x.5);");
} catch (e) {
    print (e.name);
}
