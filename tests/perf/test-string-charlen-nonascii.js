/*
 *  Character length for non-ASCII strings.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var str;
    var i, n;

    str = '\u1234\u2345\u3456\u4567\u5678\u6789\u789a\u89ab\u9abc\uabcd\ubcde\ucdef\udef0\uef01\uf012\u0123';
    while (str.length < 16384) {
        str = str + str;
    }
    print(str.length);

    for (i = 0; i < 1e5; i++) {
        void (str + 'x').length;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
