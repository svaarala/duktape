// Footprint of a Promise object and its resolve/reject function objects.

function test() {
    var promises = [];
    var resolves = [];
    var rejects = [];
    var executor = function (resolve, reject) { resolves.push(resolve); rejects.push(reject); };

    while (promises.length < 1e5) {
        if ((promises.length % 100) == 0) {
            Duktape.gc();
        }
        promises.push(new Promise(executor));
    }
    Duktape.gc();

    print(promises.length + ' Promises created');
}

test();
