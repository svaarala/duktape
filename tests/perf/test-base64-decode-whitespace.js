if (typeof print !== 'function') { print = console.log; }

function test() {
    var tmp1 = [];
    var tmp2 = [];
    var tmp3 = [];
    var i, n, buf;

    print('build');
    buf = Duktape.Buffer(1024);
    for (i = 0; i < 1024; i++) {
        buf[i] = Math.random() * 256;
    }
    tmp1 = String(buf);
    for (i = 0; i < 1024; i++) {
        tmp2.push(tmp1);
    }
    tmp2 = tmp2.join('');
    tmp2 = Duktape.enc('base64', tmp2);

    // add newlines, intentionally not a multiple of 4
    for (i = 0; i < tmp2.length; i += 77) {
       tmp3.push(tmp2.substring(i, i + 77));
    }
    tmp2 = tmp3.join('\n') + '\n';

    print(tmp2.length);
    print('run');
    for (i = 0; i < 2000; i++) {
        // Assigning to 'res' avoids garbage collection of result; this is
        // intentional to avoid mixing string intern performance to the test.
        var res = Duktape.dec('base64', tmp2);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
