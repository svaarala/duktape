function test() {
    var tmp1 = [];
    var tmp2 = [];
    var i, n;

    print('build');
    for (i = 0; i < 1024; i++) {
        tmp1.push(Math.floor(Math.random() * 16).toString(16))
    }
    tmp1 = tmp1.join('');
    for (i = 0; i < 1024; i++) {
        tmp2.push(tmp1);
    }
    tmp2 = tmp2.join('');

    print(tmp2.length);
    print('run');
    for (i = 0; i < 10000; i++) {
        Duktape.dec('hex', tmp2);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
