function test() {
    var src = [];
    var i;
    var fun;

    src.push('(function outer() {');
    for (i = 0; i < 100; i++) {
        src.push('    function inner' + i + '() {}');
    }
    src.push('})');

    //print(src.join('\n'));
    fun = eval(src.join('\n'));

    for (i = 0; i < 1e4; i++) {
        fun();
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
