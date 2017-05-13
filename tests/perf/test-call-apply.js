if (typeof print !== 'function') { print = console.log; }

function target(x) {
}

function test() {
    var f = target;
    var i;
    var args = [ 123 ];

    for (i = 0; i < 1e6; i++) {
        target.apply(null, args);
        target.apply(null, args);
        target.apply(null, args);
        target.apply(null, args);
        target.apply(null, args);
        target.apply(null, args);
        target.apply(null, args);
        target.apply(null, args);
        target.apply(null, args);
        target.apply(null, args);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
