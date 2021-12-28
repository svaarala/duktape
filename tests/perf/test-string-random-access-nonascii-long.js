if (typeof print !== 'function') { print = console.log; }

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
    var dummy = [];
    while (dummy.length < 0x10000) {
        dummy.push(String.fromCharCode(dummy.length));
    }
    print(dummy.length);

    var strlen = str.length;
    for (i = 0; i < 250; i++) {
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

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
