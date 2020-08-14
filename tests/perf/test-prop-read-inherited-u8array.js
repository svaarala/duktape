/*
 *  Basic property read performance for an inherited Uint8Array property
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var root = new Uint8Array(10000);
    var i;
    var ign;
    var obj = Object.create(root);
    obj = Object.create(obj);  // two levels of inheritance
    if (typeof Duktape !== 'undefined') { Duktape.compact(root); Duktape.compact(obj); }

    for (i = 0; i < 1e7; i++) {
        ign = obj[1234];
        ign = obj[1234];
        ign = obj[1234];
        ign = obj[1234];
        ign = obj[1234];
        ign = obj[1234];
        ign = obj[1234];
        ign = obj[1234];
        ign = obj[1234];
        ign = obj[1234];
    }
    print(ign);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
