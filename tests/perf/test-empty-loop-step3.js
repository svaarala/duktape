if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;

    for (i = 0; i < 3e8; i += 3) {
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
