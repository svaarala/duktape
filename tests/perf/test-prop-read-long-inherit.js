function test() {
    var i;
    var obj;

    obj = { foo: 123 };
    for (i = 0; i < 100; i++) {
        obj = Object.create(obj);
    }

    for (i = 0; i < 1e7; i++) {
        void obj.foo;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
