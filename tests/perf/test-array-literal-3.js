/*
 *  Create Array using a literal
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var arr;
    var i;

    for (i = 0; i < 1e6; i++) {
        arr = [ 'foo', 'bar', 'quux' ];
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
