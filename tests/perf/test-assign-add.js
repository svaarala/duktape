if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var t;
    var a = 10, b = 20;

    for (i = 0; i < 1e7; i++) {
        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;
        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;

        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;
        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;

        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;
        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;

        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;
        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;

        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;
        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;

        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;
        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;

        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;
        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;

        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;
        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;

        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;
        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;

        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;
        t = a + b; t = a + b; t = a + b; t = a + b; t = a + b;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
