if (typeof print !== 'function') { print = console.log; }


function test() {
    var i;  // is in outer scope, Duktape 2.x accesses using slow path

    function inner() {
        for (i = 0; i < 1e7; i++) {
        }
    }
    inner();
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
