/*
 *  Recursion depth book-keeping bug for JSON.stringify() fast path
 *  when unboxing objects.  When the fast path code unboxes e.g. a
 *  String to encode the plain string, it doesn't decrease recursion
 *  depth.
 *
 *  This bug has no outward effect except for being slower than intended:
 *  the fast path reaches an internal recursion limit and falls back to
 *  the slow path which works correctly.
 */

/*===
80001
===*/

function test() {
    var val = [];
    for (var i = 0; i < 10000; i++) {
        val.push(new String('dummy'));
    }

    var res = JSON.stringify(val);
    print(res.length);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
