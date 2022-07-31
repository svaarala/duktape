/*
 *  Reading a variable.
 */

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

test();
