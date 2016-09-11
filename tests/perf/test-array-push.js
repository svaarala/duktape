/*
 *  Basic array.push() performance
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var i, j;
    var arr;

    for (i = 0; i < 1e5; i++) {
        arr = [];
        for (j = 0; j < 10; j++) {
            arr.push('foo'); arr.push('bar');
            arr.push('foo'); arr.push('bar');
            arr.push('foo'); arr.push('bar');
            arr.push('foo'); arr.push('bar');
            arr.push('foo'); arr.push('bar');
        }
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
