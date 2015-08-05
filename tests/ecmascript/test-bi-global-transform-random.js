/*===
0
10
20
30
40
50
60
70
80
90
===*/

/* Random input strings test.  No output verification, useful for valgrind etc.
 * Exercises escape(), unescape(), encodeURI(), decodeURI(), encodeURIComponent(),
 * decodeURIComponent().
 */

function randomStringTest() {
    var i, j, len, arr, str;

    for (i = 0; i < 100; i++) {
        if ((i % 10) == 0) { print(i); }

        len = Math.floor(Math.random() * 40000);

        arr = [];
        for (j = 0; j < len; j++) {
            // restrict to bmp
            arr.push(Math.floor(Math.random() * 0x10000));
        }
        str = String.fromCharCode.apply(null, arr);
        //print(len, str.length, Duktape.Buffer(str).length);

        [ escape, unescape, encodeURI, decodeURI, encodeURIComponent, decodeURIComponent ].forEach(function (fn) {
            try {
                var tmp = fn(str);
            } catch (e) {
                // we don't really care what happens, this is just for covering code paths
                // for memory safety
            }
        });
    }
}

try {
    randomStringTest();
} catch (e) {
    print(e.stack || e);
}
