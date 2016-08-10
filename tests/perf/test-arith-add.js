if (typeof print !== 'function') { print = console.log; }

function test() {
    var x, y, z;
    var i, n;

    x = 0xdeadbeef;
    y = 0xcafed00d;

    for (i = 0, n = 4e7; i < n; i++) {
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
