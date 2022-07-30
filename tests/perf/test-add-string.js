function test() {
    var i;
    var x = 'foo';
    var t;

    for (i = 0; i < 1e7; i++) {
        t = x + 'bar';
        t = x + 'bar';
        t = x + 'bar';
        t = x + 'bar';
        t = x + 'bar';
        t = x + 'bar';
        t = x + 'bar';
        t = x + 'bar';
        t = x + 'bar';
        t = x + 'bar';
    }
}

test();
