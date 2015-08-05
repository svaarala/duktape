/*
 *  Exercise different string lengths to cover duk_lexer.c accumulation
 *  buffer handling.  The lexer reuses an underlying buffer (and bufwriter)
 *  for tokenization, exercise the resizing behavior of that buffer by
 *  parsing an eval() which concatenates two strings of random length.
 */

/*===
build
run
0
1000
2000
3000
4000
5000
6000
7000
8000
9000
done
===*/

function test() {
    var i, j, k;
    var str = [];

    print('build');
    for (i = 0; i < 10000; i++) {
        str[i] = 'x';
    }
    str = str.join('');

    print('run');
    for (i = 0; i < 1e4; i++) {
        if ((i % 1000) == 0) { print(i); }
        j = Math.floor(Math.random() * str.length);
        k = Math.floor(Math.random() * str.length);
        var src = '"' + str.substring(0, j) + '" + "' + str.substring(0, k) + '"';
        if (eval(src).length !== j + k) { throw new Error('failed with lengths: ' + j + ', ' + k); }
    }

    print('done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
