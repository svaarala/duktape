/*
 *  Math.random() test
 */

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

test();
