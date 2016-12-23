if (typeof print !== 'function') { print = console.log; }

function test() {
    var tmp1 = [];
    var tmp2 = [];
    var i, n, buf;
    var inp1, inp2;

    print('build');
    buf = new Uint8Array(1024);
    for (i = 0; i < 1024; i++) {
        buf[i] = Math.random() * 128;  // restrict to ASCII
    }
    tmp1 = new TextDecoder().decode(buf);
    print(tmp1.length);
    for (i = 0; i < 1024; i++) {
        tmp2.push(tmp1);
    }
    tmp2 = new TextEncoder().encode(tmp2.join(''));
    print(tmp2.length);

    print('run');
    inp1 = Duktape.enc('jx', { foo: tmp2 });
    inp2 = Duktape.enc('jx', { foox: tmp2 });

    for (i = 0; i < 1000; i++) {
        var res1 = Duktape.dec('jx', inp1);
        var res2 = Duktape.dec('jx', inp2);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
