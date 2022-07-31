/*
 *  Basic function call performance, constructor call through a register.
 */

function test() {
    var i;

    function f() { return; }

    var t1 = Date.now();

    for (i = 0; i < 2e5; i++) {
        new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f();
        new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f();
        new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f();
        new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f();
        new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f();
        new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f();
        new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f();
        new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f();
        new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f();
        new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f(); new f();
    }

    var t2 = Date.now();
    print((2e5 * 100 / (t2 - t1)) + ' calls per millisecond');
}

test();
