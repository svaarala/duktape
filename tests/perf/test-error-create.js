/*
 *  Create Error creation (tracedata)
 */

if (typeof print !== 'function') { print = console.log; }

// Avoid tailcalls on purpose to ensure traceback is reasonably long.

function fun0() {
    var res = new Error('aiee');
    return res;
}

function fun1() {
    var res = fun0();
    return res;
}

function fun2() {
    var res = fun1();
    return res;
}

function fun3() {
    var res = fun2();
    return res;
}

function fun4() {
    var res = fun3();
    return res;
}

function test() {
    var err;
    var i;

    for (i = 0; i < 1e6; i++) {
        err = fun4();
    }

    print(err.stack || err);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
