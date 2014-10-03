/*
 *  Regexp bug reported by Conrad Pankoff.
 *
 *  Duktape 0.9.0 compiled /(?:a)?/ incorrectly.  The regexp bytecode was
 *  essentially matching /a(?:)?/ instead, because the "previous atom"
 *  state was incorrectly updated for non-capturing groups.
 */

/*===
false
false
false
true
true
true
true
true
true
true
true
true
===*/

function regexpNoncapturingTest() {
    // should be false
    print(/a/.test("x"));
    print(/(a)/.test("x"));
    print(/(?:a)/.test("x"));

    // should be true
    print(/a/.test("a"));
    print(/(a)/.test("a"));
    print(/(?:a)/.test("a"));

    // should be true
    print(/a?/.test("x"));
    print(/(a)?/.test("x"));
    print(/(?:a)?/.test("x"));

    // should be true
    print(/a?/.test("a"));
    print(/(a)?/.test("a"));
    print(/(?:a)?/.test("a"));
}

try {
    regexpNoncapturingTest();
} catch (e) {
    print(e);
}
