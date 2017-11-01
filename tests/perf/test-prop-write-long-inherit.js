function test() {
    var i;
    var obj;

    obj = { foo: 123 };
    for (i = 0; i < 100; i++) {
        obj = Object.create(obj);
    }

    for (i = 0; i < 1e7; i++) {
        // Because a write always causes an own property to be created,
        // there's no difference between a shallow and deep object except
        // on the very first write.
        obj.foo = 123;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
