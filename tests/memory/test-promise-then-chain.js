// Footprint of an unresolved long .then() chain.

function test() {
    var count = 1e3;
    function id(v) { return v; }

    // Root and entire chain will remain unresolved.
    var root = new Promise(function () {});
    var curr = root;

    for (var i = 0; i < count; i++) {
        if ((i % 100) == 0) {
            Duktape.gc();
        }
        curr = curr.then(id);
    }
    Duktape.gc();

    print(count + ' Promises created');
}

test();
