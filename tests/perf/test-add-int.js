if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var x = 123;
    var t;

    for (i = 0; i < 1e7; i++) {
        t = x + 3;
        t = x + 3;
        t = x + 3;
        t = x + 3;
        t = x + 3;
        t = x + 3;
        t = x + 3;
        t = x + 3;
        t = x + 3;
        t = x + 3;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
