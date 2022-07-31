/*
 *  Important fast path: character length for pure ASCII strings.
 */

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

test();
