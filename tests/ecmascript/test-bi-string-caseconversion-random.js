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
 * Exercises fromCharCode(), toUpperCase(), toLowerCase(), toLocaleUpperCase(),
 * toLocaleLowerCase().
 */

function randomStringTest() {
    var i, j, len, arr, str, res1, res2, res3;

    for (i = 0; i < 100; i++) {
        if ((i % 10) == 0) { print(i); }

        len = Math.floor(Math.random() * 40000);

        arr = [];
        for (j = 0; j < len; j++) {
            arr.push(Math.floor(Math.random() * 0x1000000));
        }
        str = String.fromCharCode.apply(null, arr);
        //print(len, str.length, Duktape.Buffer(str).length);

        res1 = str.toUpperCase();
        res2 = str.toLowerCase();
        res3 = res1.toLowerCase();

        res1 = str.toLocaleUpperCase();
        res2 = str.toLocaleLowerCase();
        res3 = res1.toLocaleLowerCase();

        // Can't really compare result lengths or contents; case conversion
        // may change string length, and conversion "route" affects final
        // result.  This is just a brute force coverage test.
    }
}

try {
    randomStringTest();
} catch (e) {
    print(e.stack || e);
}
