if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var x = 123;
    var y = 234;
    var t;

    for (i = 0; i < 1e6; i++) {
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
        t = x + y;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
