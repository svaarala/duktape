function test() {
    var i;
    var s = 'x';
    var t;

    for (i = 0; i < 5e5; i++) {
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
        t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y'; t = s + 'y';
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
