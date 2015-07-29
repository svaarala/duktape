/*
 *  Stringify strings of various lengths; exercises bufwriter
 *  handling.
 */

/*===
done
===*/

function test() {
    var str = '';
    var i, j;

    for (i = 0; i < 1024; i++) {
        str += 'x';

        for (j = 0; j < 256; j++) {
            void JSON.stringify(str);
        }
    }

    print('done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
