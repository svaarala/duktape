// Footprint of a pending Promise object, no resolve/reject functions.

function test() {
    var promises = [];
    function nop() {}

    while (promises.length < 1e5) {
        if ((promises.length % 100) == 0) {
            Duktape.gc();
        }
        promises.push(new Promise(nop));
    }
    Duktape.gc();

    print(promises.length + ' Promises created');
}

test();
