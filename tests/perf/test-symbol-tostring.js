if (typeof print !== 'function') { print = console.log; }

function test() {
    var S = Symbol('foo');
    var i;

    for (i = 0; i < 1e6; i++) {
        String(S);
        String(S);
        String(S);
        String(S);
        String(S);
        String(S);
        String(S);
        String(S);
        String(S);
        String(S);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
