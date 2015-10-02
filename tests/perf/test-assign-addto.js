if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var t = 0;
    var a = 10;

    for (i = 0; i < 1e7; i++) {
        t += a; t += a; t += a; t += a; t += a;
        t += a; t += a; t += a; t += a; t += a;

        t += a; t += a; t += a; t += a; t += a;
        t += a; t += a; t += a; t += a; t += a;

        t += a; t += a; t += a; t += a; t += a;
        t += a; t += a; t += a; t += a; t += a;

        t += a; t += a; t += a; t += a; t += a;
        t += a; t += a; t += a; t += a; t += a;

        t += a; t += a; t += a; t += a; t += a;
        t += a; t += a; t += a; t += a; t += a;

        t += a; t += a; t += a; t += a; t += a;
        t += a; t += a; t += a; t += a; t += a;

        t += a; t += a; t += a; t += a; t += a;
        t += a; t += a; t += a; t += a; t += a;

        t += a; t += a; t += a; t += a; t += a;
        t += a; t += a; t += a; t += a; t += a;

        t += a; t += a; t += a; t += a; t += a;
        t += a; t += a; t += a; t += a; t += a;

        t += a; t += a; t += a; t += a; t += a;
        t += a; t += a; t += a; t += a; t += a;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
