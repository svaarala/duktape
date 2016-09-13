/*
 *  Output a 1024x1024 image generated using Math.random().
 */

function test(filename, threshold) {
    var i, j, t;
    var res = [];

    res.push('P2\n1024 1024\n255\n');

    for (i = 0; i < 1024; i++) {
        for (j = 0; j < 1024; j++) {
            t = Math.random();
            if (typeof threshold === 'number') {
                res.push(t >= threshold ? '255 ' : '0 ');
            } else {
                res.push(String(Math.floor(t * 256.0)) + ' ');
            }
        }
        res.push('\n');
    }

    res = res.join('');
    writeFile(filename, res);
    print('Wrote', filename);
}

try {
    test('rnd-threshold-01.pgm', 0.1);
    test('rnd-threshold-05.pgm', 0.5);
    test('rnd-threshold-09.pgm', 0.9);
    test('rnd-threshold-none.pgm', null);
} catch (e) {
    print(e.stack || e);
}
