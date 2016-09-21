function test() {
    var i;
    var src = '[a-z]foo.bar.quux.baz{1,3}[a-zA-Z0-9]+$';
    var re;

    for (i = 0; i < 1e4; i++) {
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
        re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src); re = new RegExp(src);
    }
}
try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
