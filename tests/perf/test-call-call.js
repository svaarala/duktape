if (typeof print !== 'function') { print = console.log; }

function target(x) {
}

function test() {
    var f = target;
    var i;

    for (i = 0; i < 1e6; i++) {
        target.call(null, 123);
        target.call(null, 123);
        target.call(null, 123);
        target.call(null, 123);
        target.call(null, 123);
        target.call(null, 123);
        target.call(null, 123);
        target.call(null, 123);
        target.call(null, 123);
        target.call(null, 123);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
