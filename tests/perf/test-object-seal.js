if (typeof print !== 'function') { print = console.log; }

function test() {
    var obj;
    var i;
    var os = Object.seal;

    for (i = 0; i < 1e6; i++) {
        obj = { prop1: 1, prop2: 2, prop3: 3, prop4: 4, prop5: 5,
                prop6: 6, prop7: 7, prop8: 8, prop9: 9, prop10: 10 };
        os(obj);
        obj = [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 ];
        os(obj);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
