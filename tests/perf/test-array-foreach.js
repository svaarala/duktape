/*
 *  Basic array.forEach() performance
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var arr = [];
    var i;

    for (i = 0; i < 1e3; i++) {
        arr.push('value-' + i);
    }

    for (i = 0; i < 2e4; i++) {
        arr.forEach(function () {});
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
