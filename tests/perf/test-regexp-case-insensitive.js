/*
 *  Case insensitive regexp compilation has bad performance behavior at
 *  least up to Duktape 1.3.0.  Basic test for that behavior.
 */
function test() {
    var i;
    var src = [];
    for (i = 0; i < 1e2; i++) {
        // Use a RegExp constructor call rather than a literal to ensure the
        // RegExp is compiled on every loop.
        var re = new RegExp('[\\u0000-\\uffff]', 'i');
    }
}
try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
