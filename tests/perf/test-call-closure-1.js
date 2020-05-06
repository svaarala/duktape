/*
 *  Nested function causes an actual scope object to be
 *  created and closed.
 */

if (typeof print !== 'function') { print = console.log; }

function func() {
    var v0 = true;
    var v1 = true;
    var v2 = true;
    var v3 = true;
    var v4 = true;
    var v5 = true;
    var v6 = true;
    var v7 = true;
    var v8 = true;
    var v9 = true;

    // This forces the the scope of 'func' to be preserved
    // when func() is closed.
    return function (x) { return eval(x); };
}

function test() {
    var i;
    var ign;

    for (i = 0; i < 1e5; i++) {
        ign = func();
        ign = func();
        ign = func();
        ign = func();
        ign = func();
        ign = func();
        ign = func();
        ign = func();
        ign = func();
        ign = func();
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
