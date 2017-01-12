if (typeof print !== 'function') { print = console.log; }

function test() {
    var arr = [];
    for (var i = 0; i < 1e6; i++) {
        arr.push(String(i));
    }
    print(arr[1e6-1]);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
