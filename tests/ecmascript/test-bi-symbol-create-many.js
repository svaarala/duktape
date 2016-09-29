/*
 *  Create 10M symbols.
 */

/*===
done
===*/

function test() {
    var i, s;

    for (i = 0; i < 1e7; i++) {
        s = Symbol('foo');
    }
    print('done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
