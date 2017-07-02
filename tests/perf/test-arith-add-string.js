if (typeof print !== 'function') { print = console.log; }

function test() {
    var x, y, z;
    var i, n;

    x = 'foo';
    y = 'bar';

    for (i = 0, n = 1e6; i < n; i++) {
        z = x + y;
        z = x + y;
        z = x + y;
        z = x + y;
        z = x + y;
        z = x + y;
        z = x + y;
        z = x + y;
        z = x + y;
        z = x + y;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
