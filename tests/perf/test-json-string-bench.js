if (typeof print !== 'function') { print = console.log; }

function rndAscii(len) {
    var res = [];
    var i;
    for (i = 0; i < len; i++) {
        if (Math.random() < 0.08) {
            res.push(String.fromCharCode(Math.floor(Math.random() * 128)));
        } else {
            res.push(String.fromCharCode(Math.floor(Math.random() * 0x5f) + 0x20));
        }
    }
    return res.join('');
}

function test() {
    var sz = 1;

    for (var i = 0; i <= 20; i++) {
        var s = rndAscii(sz);
        var obj = { key: s };
        var start = Date.now();
        var ign = JSON.stringify(obj);
        //print(ign);
        var end = Date.now();
        var diff = end - start;
        print(sz, start, end, diff,
              diff * 1000 / sz,                     // microseconds/byte
              (sz / (1024 * 1024)) / (diff / 1000)  // megabytes/second
              );
        sz *= 2;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
