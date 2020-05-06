if (typeof print !== 'function') { print = console.log; }

function test() {
    var gopd = Object.getOwnPropertyDescriptor;
    var i;
    var obj = { foo: 123, bar: 234, quux: 345 };
    var arr = [ 1, 2, 3, 4 ];

    for (i = 0; i < 1e6; i++) {
        void gopd(obj, 'foo');
        void gopd(obj, 'bar');
        void gopd(obj, 'quux');
        void gopd(obj, 'nosuch');
        void gopd(arr, 0);
        void gopd(arr, 1);
        void gopd(arr, 2);
        void gopd(arr, 3);
        void gopd(arr, 4);
        void gopd(arr, 'length');
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
