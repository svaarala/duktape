if (typeof print !== 'function') { print = console.log; }

function test() {
    var odp = Object.defineProperty;
    //odp = Reflect.defineProperty;
    var i;
    var obj = { foo: 123, bar: 234, quux: 345 };
    var arr = [ 1, 2, 3, 4 ];
    var desc1 = { value: 1001, writable: false, enumerable: true, configurable: true };
    var desc2 = { value: 1001 };

    for (i = 0; i < 1e6; i++) {
        void odp(obj, 'foo', desc1);
        void odp(obj, 'bar', desc1);
        void odp(obj, 'quux', desc1);
        void odp(obj, 'nosuch', desc1);
        void odp(obj, 'nosuch2', desc1);
        void odp(arr, 0, desc2);
        void odp(arr, 1, desc2);
        void odp(arr, 2, desc2);
        void odp(arr, 3, desc2);
        void odp(arr, 4, desc2);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
