function test() {
    var i;
    var t = '';

    for (i = 0; i < 1e4; i++) {
        t = t + 'x';
        t = t + 'x';
        t = t + 'x';
        t = t + 'x';
        t = t + 'x';
        t = t + 'x';
        t = t + 'x';
        t = t + 'x';
        t = t + 'x';
        t = t + '\ucafe';
    }
}

test();
