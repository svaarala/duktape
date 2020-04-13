if (typeof print !== 'function') { print = console.log; }

function test() {
    var S1 = Symbol('s1');
    var S2 = Symbol('s2');
    var S3 = Symbol('s3');
    var S4 = Symbol('s4');
    var S5 = Symbol('s5');
    var i;
    var obj = {};
    obj[0] = 'zero';
    obj[1] = 'one';
    obj[2] = 'two';
    obj[3] = 'three';
    obj['foo'] = 'foo';
    obj['bar'] = 'bar';
    obj['quux'] = 'quux';
    obj['baz'] = 'baz';
    obj[S2] = 's2';
    obj[S3] = 's3';
    obj[S1] = 's1';
    obj[S5] = 's5';
    obj[S4] = 's4';

    var tmp = Reflect.ownKeys(obj);
    tmp.forEach(function (v, i) {
        print(i, String(v));
    });
    [ '0', '1', '2', '3', 'foo', 'bar', 'quux', 'baz', S2, S3, S1, S5, S4 ].forEach(function (v, i) {
        if (tmp[i] !== v) {
            print('INVALID ORDER at index ' + i);
        }
    });

    for (i = 0; i < 1e5; i++) {
        void Reflect.ownKeys(obj);
        void Reflect.ownKeys(obj);
        void Reflect.ownKeys(obj);
        void Reflect.ownKeys(obj);
        void Reflect.ownKeys(obj);
    }
}
try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
