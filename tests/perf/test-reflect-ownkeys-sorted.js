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
