if (typeof print !== 'function') { print = console.log; }

function test() {
    var b = new Float32Array(4096);
    var i;

    print(typeof b);

    for (i = 0; i < 1e7; i++) {
        // Standard in-range double-to-float cast.
        b[100] = 123.456;
        b[100] = 123.456;
        b[100] = 123.456;
        b[100] = 123.456;
        b[100] = 123.456;
        b[100] = 123.456;
        b[100] = 123.456;
        b[100] = 123.456;
        b[100] = 123.456;
        b[100] = 123.456;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
