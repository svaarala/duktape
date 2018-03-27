// Footprint of a resolved Promise object, no resolve/reject functions.

function test() {
    var promises = [];

    while (promises.length < 1e5) {
        if ((promises.length % 100) == 0) {
            Duktape.gc();
        }
        promises.push(Promise.resolve(123));
    }
    Duktape.gc();

    print(promises.length + ' Promises created');
}

test();
