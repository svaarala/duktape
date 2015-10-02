if (typeof print !== 'function') { print = console.log; }

function test() {
    var arr = [];
    for (var i = 0; i < 1e3; i++) {
        arr[i] = i;
    }
    for (var i = 0; i < 1e4; i++) {
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
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
