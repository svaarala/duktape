/*
 *  Basic Proxy 'apply' trap performance.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;

    function f_target() { return; }  // ignored

    var f = new Proxy(f_target, {
        apply: function (targ, thisArg, argArray) {
            return;
        }
    });

    var t1 = Date.now();

    for (i = 0; i < 1e6; i++) {
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
        f(); f(); f(); f(); f(); f(); f(); f(); f(); f();
    }

    var t2 = Date.now();
    print((1e6 * 100 / (t2 - t1)) + ' calls per millisecond');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
