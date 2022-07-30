/*
 *  Basic function call performance, call through a variable lookup.
 */

var func;

function test() {
    var i;

    function f() { return; }

    func = f;

    var t1 = Date.now();

    for (i = 0; i < 4e5; i++) {
        func(); func(); func(); func(); func(); func(); func(); func(); func(); func();
        func(); func(); func(); func(); func(); func(); func(); func(); func(); func();
        func(); func(); func(); func(); func(); func(); func(); func(); func(); func();
        func(); func(); func(); func(); func(); func(); func(); func(); func(); func();
        func(); func(); func(); func(); func(); func(); func(); func(); func(); func();
        func(); func(); func(); func(); func(); func(); func(); func(); func(); func();
        func(); func(); func(); func(); func(); func(); func(); func(); func(); func();
        func(); func(); func(); func(); func(); func(); func(); func(); func(); func();
        func(); func(); func(); func(); func(); func(); func(); func(); func(); func();
        func(); func(); func(); func(); func(); func(); func(); func(); func(); func();
    }

    var t2 = Date.now();
    print((4e5 * 100 / (t2 - t1)) + ' calls per millisecond');
}

test();
