/*
 *  Test JSON string parsing with various input lengths;
 *  exercises bufwriter handling, intended for asserts and
 *  valgrinding.
 */

/*===
done
===*/

function test() {
    var str = '';
    var tmp;
    var i, j;

    for (i = 0; i < 1024; i++) {
        str += 'x';
        tmp = JSON.stringify(str);

        for (j = 0; j < 256; j++) {
            void JSON.parse(tmp);
        }
    }

    print('done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
