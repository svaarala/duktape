if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var x = 'foo';
    var t;

    for (i = 0; i < 1e7; i++) {
        t = x + 'bar';
        t = x + 'bar';
        t = x + 'bar';
        t = x + 'bar';
        t = x + 'bar';
        t = x + 'bar';
        t = x + 'bar';
        t = x + 'bar';
        t = x + 'bar';
        t = x + 'bar';
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
