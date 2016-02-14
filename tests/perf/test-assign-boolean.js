/*
 *  Loading boolean constant to register.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var t;

    for (i = 0; i < 1e7; i++) {
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;

        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;

        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;

        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;

        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;

        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;

        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;

        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;

        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;

        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
        t = true; t = false;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
