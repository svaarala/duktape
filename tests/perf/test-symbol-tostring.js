function test() {
    var S = Symbol('foo');
    var i;

    for (i = 0; i < 1e6; i++) {
        String(S);
        String(S);
        String(S);
        String(S);
        String(S);
        String(S);
        String(S);
        String(S);
        String(S);
        String(S);
    }
}

test();
