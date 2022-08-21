/*
 *  This test from https://github.com/kangax/compat-table succeeds but
 *  should fail.
 */

/*===
SyntaxError
SyntaxError
===*/

try {
    var t = eval('/(?P<name>a)(?P=name)/');
    print(t);
    print(t.test("aa"));
} catch (e) {
    print(e.name);
}

try {
    // In General '(?' followed by something else than ':' is a SyntaxError.
    var t = eval('/(?X)/');
    print(t);
} catch (e) {
    print(e.name);
}
