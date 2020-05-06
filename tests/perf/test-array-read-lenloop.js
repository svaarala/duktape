/*
 *  Basic array read loop performance, loop to .length
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var arr = [];
    var ign;
    var i, j;

    while (arr.length < 1000) {
        arr.push('foo');
    }

    for (i = 0; i < 3e4; i++) {
        // Typical loop idiom where index check is against .length.
        for (j = 0; j < arr.length; j++) {
            ign = arr[j];
        }
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
