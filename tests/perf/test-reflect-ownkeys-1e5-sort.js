if (typeof print !== 'function') { print = console.log; }

function test() {
    var obj = {};
    var i;
    var t;

    // Insert index keys to an object; these end up in the index part,
    // in no particular order.
    print('build');

    t = 0;
    for (i = 0; i < 1e5; i++) {
        obj[t] = true;
        t = (t + 123457) % 1e5;
    }

    print('get once');
    var tmp = Reflect.ownKeys(obj);

    print('check order');
    for (i = 0; i < tmp.length - i; i++) {
        if (Number(tmp[i]) >= Number(tmp[i + 1])) {
            print('INVALID ORDER at index ' + i);
        }
    }

    print('test loop');  // total 2 times
    for (i = 0; i < 1; i++) {
        print(i);
        void Reflect.ownKeys(obj);
    }
}
try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
