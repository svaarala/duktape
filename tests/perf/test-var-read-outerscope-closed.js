/*
 *  Reading a variable.
 */

function test() {
    function outer() {
        var o = 123;

        return function inner() {
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
        };
    }

    var f = outer();
    f();
}

test();
