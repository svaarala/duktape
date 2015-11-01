/*
 *  Important fast path: character length for pure ASCII strings.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var str;
    var i, n;

    str = 'aAbBcCdDeEfFgGhH';
    while (str.length < 65536) {
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
