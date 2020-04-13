if (typeof print !== 'function') { print = console.log; }

function mkobj(depth) {
    var res = { foo: [ 1, 2, 'bar' ], bar: Math.cos };
    var i;
    if (depth <= 0) {
        return res;
    }
    for (i = 0; i < 5; i++) {
        res['key-' + i] = mkobj(depth - 1);
    }
    for (i = 0; i < 5; i++) {
        res[i] = mkobj(depth - 1);
    }
    return res;
}

function test() {
    var obj = mkobj(4);
    var i;

    //print(JSON.stringify(obj));

    for (i = 0; i < 1e4; i++) {
        Duktape.gc();
    }
    print('done');
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
