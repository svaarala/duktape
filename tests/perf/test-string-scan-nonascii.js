function test() {
    var str = [];
    var i;

    for (i = 0; i < 1024; i++) {
        str.push(String.fromCharCode(i & 0xffff));
    }
    str = str.join('');

    /* Because this testcase is not about interning, intern the substrings
     * to avoid string table traffic in this particular test.
     */
    var dummy = [
        str.charCodeAt(0),
        str.charCodeAt(1023),
        str.charCodeAt(100),
        str.charCodeAt(200),
        str.charCodeAt(300),
        str.charCodeAt(400),
        str.charCodeAt(900),
        str.charCodeAt(800),
        str.charCodeAt(700),
        str.charCodeAt(600),
        str.charCodeAt(500)
    ];

    for (i = 0; i < 1e6; i++) {
        void str.charCodeAt(0);
        void str.charCodeAt(1023);
        void str.charCodeAt(100);
        void str.charCodeAt(200);
        void str.charCodeAt(300);
        void str.charCodeAt(400);
        void str.charCodeAt(900);
        void str.charCodeAt(800);
        void str.charCodeAt(700);
        void str.charCodeAt(600);
        void str.charCodeAt(500);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
