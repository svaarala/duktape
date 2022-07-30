function test() {
    var str = [];
    var i;

    for (i = 0; i < 1024; i++) {
        str.push(i & 0x7f);
    }
    str = String.fromCharCode.apply(null, str);
    print(str.length);

    var prefix1 = str.substr(0, 10);
    var prefix2 = prefix1 + '_';  // No match.

    for (i = 0; i < 1e6; i++) {
        void str.startsWith(prefix1);
        void str.startsWith(prefix2);
        void str.startsWith(prefix1);
        void str.startsWith(prefix2);
        void str.startsWith(prefix1);
        void str.startsWith(prefix2);
        void str.startsWith(prefix1);
        void str.startsWith(prefix2);
        void str.startsWith(prefix1);
        void str.startsWith(prefix2);
    }
}

test();
