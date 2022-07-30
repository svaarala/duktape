function test() {
    var str = [];
    var i;

    for (i = 0; i < 1024; i++) {
        str.push(i & 0xffff);
    }
    str = String.fromCharCode.apply(null, str);
    print(str.length);

    /* Because this testcase is not about interning, intern the substrings
     * to avoid string table traffic in this particular test.
     */
    var dummy = [];
    while (dummy.length < 0x10000) {
        dummy.push(String.fromCharCode(dummy.length));
    }
    print(dummy.length);

    var strlen = str.length;
    for (i = 0; i < 1e6; i++) {
        void str.charCodeAt(Math.random() * strlen);
        void str.charCodeAt(Math.random() * strlen);
        void str.charCodeAt(Math.random() * strlen);
        void str.charCodeAt(Math.random() * strlen);
        void str.charCodeAt(Math.random() * strlen);
        void str.charCodeAt(Math.random() * strlen);
        void str.charCodeAt(Math.random() * strlen);
        void str.charCodeAt(Math.random() * strlen);
        void str.charCodeAt(Math.random() * strlen);
        void str.charCodeAt(Math.random() * strlen);
    }
}

test();
