if (typeof print !== 'function') { print = console.log; }

function test() {
    var odp = Object.defineProperties;
    var i;
    var obj = { foo: 123, bar: 234, quux: 345 };
    var arr = [ 1, 2, 3, 4 ];
    var desc1 = { value: 1001, writable: false, enumerable: true, configurable: true };
    var desc2 = { value: 1001 };

    var descs1 = {
        foo: desc1,
        bar: desc1,
        quux: desc1,
        baz: desc1,  // non-existent
        quuux: desc2  // non-existent
    };
    var descs2 = {
        0: desc2,
        1: desc2,
        2: desc2,
        3: desc2,
        4: desc2  // non-existent, causes abandon
    };

    for (i = 0; i < 1e5; i++) {
        void odp(obj, descs1);
        void odp(arr, descs2);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
