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

/* Random replace test.  No output verification, useful for valgrind etc.
 */

function randomStringTest() {
    var i, j, len, arr, str, res;

    var longstring = 'x';
    while (longstring.length < 1e6) { longstring = longstring + longstring; }

    for (i = 0; i < 100; i++) {
        if ((i % 10) == 0) { print(i); }

        len = Math.floor(Math.random() * 100000);

        arr = [];
        for (j = 0; j < len; j++) {
            if (Math.random() < 0.5) {
                // 50% of characters are [a-z] which are used as replace matches
                arr.push(0x61 + Math.floor(Math.random() * 26));
            } else {
                arr.push(Math.floor(Math.random() * 0x1000000));
            }
        }
        str = String.fromCharCode.apply(null, arr);
        //print(len, str.length, Duktape.Buffer(str).length);

        // Try to cover all bufwriter output sites in duk_bi_string_prototype_replace()
        // with a variable replacement size.

        res = str;
        res = res.replace('a', 'FOO');
        res = res.replace('b', function () { return longstring.substring(0, Math.floor(Math.random() * 1e5)); });
        res = res.replace('c', 'foo$$');
        res = res.replace(/[a-g]+/, '$&$&$&');
        res = res.replace(/[h-m]+/, '$\'$\'$\'');
        res = res.replace(/[n-z]+/, '$`$`$`');
        res = res.replace(/([a-z])([a-z])([a-z])/, '$0$1$2$3$4');

        //print(Duktape.enc('jx', res));
    }
}

try {
    randomStringTest();
} catch (e) {
    print(e.stack || e);
}
