if (typeof print !== 'function') { print = console.log; }

function target(x) {
}

function test() {
    var f = target;
    var i;
    var args = [ 123 ];

    var t1 = Date.now();

    for (i = 0; i < 1e5; i++) {
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
        target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args); target.apply(null, args);
    }

    var t2 = Date.now();
    print((1e5 * 100 / (t2 - t1)) + ' calls per millisecond');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
