/*
 *  Math.random() test
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var rnd = Math.random;

    for (i = 0; i < 3e5; i++) {
        rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd();
        rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd();
        rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd();
        rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd();
        rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd();
        rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd();
        rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd();
        rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd();
        rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd();
        rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd(); rnd();
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
