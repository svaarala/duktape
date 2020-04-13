if (typeof print !== 'function') { print = console.log; }

function test() {
    var arr = [];
    var i;

    for (i = 0; i < 1e3; i++) {
        arr[i] = i;
    }

    var t1 = Date.now();

    for (i = 0; i < 1e4; i++) {
        // Each .forEach() call is a native call so that return 123 causes a
        // return-to-native situation which was previously a longjmp.

        arr.forEach(function () { return 123; });
        arr.forEach(function () { return 123; });
        arr.forEach(function () { return 123; });
        arr.forEach(function () { return 123; });
        arr.forEach(function () { return 123; });
        arr.forEach(function () { return 123; });
        arr.forEach(function () { return 123; });
        arr.forEach(function () { return 123; });
        arr.forEach(function () { return 123; });
        arr.forEach(function () { return 123; });
    }

    var t2 = Date.now();
    print((1e4 * 10 * arr.length / (t2 - t1)) + ' calls per millisecond');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
