function test() {
    var str = [];
    var i;

    for (i = 0; i < 65536; i++) {
        str.push(i & 0xffff);
    }
    str = String.fromCharCode.apply(null, str).repeat(64);
    print(str.length);

    /* Because this testcase is not about interning, intern the substrings
     * to avoid string table traffic in this particular test.
     */
    var dummy = [
        str.charAt(0),
        str.charAt(2023000),
        str.charAt(100000),
        str.charAt(200000),
        str.charAt(300000),
        str.charAt(400000),
        str.charAt(900000),
        str.charAt(800000),
        str.charAt(700000),
        str.charAt(600000),
        str.charAt(500000)
    ];

    for (i = 0; i < 500; i++) {
        void str.charCodeAt(0);
        void str.charCodeAt(2023000);
        void str.charCodeAt(100000);
        void str.charCodeAt(200000);
        void str.charCodeAt(300000);
        void str.charCodeAt(400000);
        void str.charCodeAt(900000);
        void str.charCodeAt(800000);
        void str.charCodeAt(700000);
        void str.charCodeAt(600000);
        void str.charCodeAt(500000);
    }
}

test();
