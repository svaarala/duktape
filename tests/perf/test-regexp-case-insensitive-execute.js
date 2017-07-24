/*
 *  Case insensitive regexp character class execution has bad performance
 *  behavior because a lot of scattered match ranges are potentially generated.
 *  Basic test for case insensitive regexp character class worst case behavior.
 */
function test() {
    var i;
    var re = new RegExp('[\\u0000-\\uffff]+', 'i');
    var t1 = Date.now();
    for (i = 0; i < 1e4; i++) {
        void re.test('foo\u1234\ucafebar\uffff\ufead');
    }
    print(((Date.now() - t1) / 1e4) + ' ms/test');
}
try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
