/*
 *  Basic enumeration performance
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var obj = { foo: 1, bar: 2, quux: 3, baz: 4, quuux: 5, 5: 'number key', 1: 'number key', 999: 'number key' };
    var arr = [ 1, 2, 3, 4, 5, 6, 7, 8 ];
    var i;
    var ign;

    for (i = 0; i < 1e5; i++) {
        for (ign in obj) {}
        for (ign in arr) {}
        for (ign in obj) {}
        for (ign in arr) {}
        for (ign in obj) {}
        for (ign in arr) {}
        for (ign in obj) {}
        for (ign in arr) {}
        for (ign in obj) {}
        for (ign in arr) {}
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
