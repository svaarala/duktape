if (typeof print !== 'function') { print = console.log; }

function test(n) {
    var obj = {};
    var i, count;

    for (i = 0; i < n; i++) {
        obj['prop' + i] = 1;
    }
    if (typeof Duktape !== 'undefined') { Duktape.compact(obj); }

    var code = [];
    code.push('(function (obj) { for (var count = 1e7; count > 0;) {');
    for (i = 0; i < 100; i++) {
        code.push('obj.prop' + (i % n) + '= 123;');
    }
    code.push('count -= 100;');
    code.push('} })');

    eval(code.join(''))(obj);
}

try {
    for (var i = 1; i <= 32; i++) {
        var t_start = typeof performance !== 'undefined' ? performance.now() : Date.now();
        test(i);
        var t_end = typeof performance !== 'undefined' ? performance.now() : Date.now();
        print(i, (t_end - t_start) / 1000);
    }
} catch (e) {
    print(e.stack || e);
    throw e;
}
