/*
 *  Reading a variable.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var o = 123;

    function inner() {
        var i;
        var t;

        for (i = 0; i < 1e6; i++) {
            t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o;
            t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o;
            t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o;
            t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o;
            t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o;
            t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o;
            t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o;
            t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o;
            t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o;
            t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o; t = o;
        }
    }

    inner();
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
