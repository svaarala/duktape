/*
 *  Basic array.forEach() performance
 */

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

test();
