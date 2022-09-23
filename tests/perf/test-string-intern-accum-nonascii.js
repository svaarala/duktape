function test() {
    var i;
    var t = '';

    for (i = 0; i < 4e3; i++) {
        t = t + '\ucafe';
        t = t + '\ufedc';
        t = t + '\ucafe';
        t = t + '\ufedc';
        t = t + '\ucafe';
        t = t + '\ufedc';
        t = t + '\ucafe';
        t = t + '\ufedc';
        t = t + '\ucafe';
        t = t + '\ufedc';
    }
}

test();
