/*
 *  Create Array using a literal
 */

function test() {
    var arr;
    var i;

    for (i = 0; i < 1e6; i++) {
        arr = [ 'foo', 'bar', 'quux' ];
    }
}

test();
