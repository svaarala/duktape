/*
 *  Create a very long refzero cascade.  Ensures that such cascades are
 *  handled without a deep C stack.
 */

/*===
build object
start cascade
end cascade
done
===*/

function test() {
    var obj;
    var i;

    print('build object');
    obj = {};
    for (i = 0; i < 1e7; i++) {
        obj = { ref: obj };
    }

    // On return 'obj' becomes unreachable and is freed.
    print('start cascade');
}

try {
    test();
    print('end cascade');
} catch (e) {
    print(e.stack || e);
}
print('done');
