function test() {
    var i;
    var src = '123 + myval';
    var myval = 234;

    for (i = 0; i < 1e6; i++) {
        void eval(src);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
