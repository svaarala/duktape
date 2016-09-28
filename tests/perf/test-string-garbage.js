function test() {
    var i;
    var s = 'x';
    var t;

    for (i = 0; i < 2e5; i++) {
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
        t = s + 'a'; t = s + 'b'; t = s + 'c'; t = s + 'd'; t = s + 'e';
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
