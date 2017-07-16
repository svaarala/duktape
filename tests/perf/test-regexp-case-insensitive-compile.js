/*
 *  Case insensitive regexp character class compilation has bad performance
 *  behavior up to Duktape 2.1.0.  In Duktape 2.2.0 there's a significant
 *  improvement but case insensitive regexps are still much slower than they
 *  could be.  Basic test for case insensitive regexp character class worst
 *  case behavior.
 */
function test() {
    var i;
    var t1 = Date.now();
    for (i = 0; i < 1e2; i++) {
        // Use a RegExp constructor call rather than a literal to ensure the
        // RegExp is compiled on every loop.
        var re = new RegExp('[\\u0000-\\uffff]', 'i');
    }
    print(((Date.now() - t1) / 1e2) + ' ms/test');
}
try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
