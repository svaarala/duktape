function test() {
    var i;
    var re = new RegExp('[\\u0000-\\uffff]+', '');
    var t1 = Date.now();
    for (i = 0; i < 1e6; i++) {
        void re.test('foo\u1234\ucafebar\uffff\ufead');
    }
    print(((Date.now() - t1) / 1e6) + ' ms/test');
}
try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
