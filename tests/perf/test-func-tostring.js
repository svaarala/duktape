if (typeof print !== 'function') { print = console.log; }

function test() {
    var fn = function foo() {};
    var i, t;

    for (i = 0; i < 1e6; i++) {
        t = fn.toString();
        t = fn.toString();
        t = fn.toString();
        t = fn.toString();
        t = fn.toString();
        t = fn.toString();
        t = fn.toString();
        t = fn.toString();
        t = fn.toString();
        t = fn.toString();
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
