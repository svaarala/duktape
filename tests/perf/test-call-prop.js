/*
 *  Basic function call performance, call through a property lookup.
 */

function test() {
    var i;

    function f() { return; }

    var obj = { prop: f };

    var t1 = Date.now();

    for (i = 0; i < 4e5; i++) {
        obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop();
        obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop();
        obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop();
        obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop();
        obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop();
        obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop();
        obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop();
        obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop();
        obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop();
        obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop(); obj.prop();
    }

    var t2 = Date.now();
    print((4e5 * 100 / (t2 - t1)) + ' calls per millisecond');
}

test();
