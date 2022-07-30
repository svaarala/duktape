function test() {
    var str = [];
    var i;

    for (i = 0; i < 1024; i++) {
        str.push(i & 0x7f);
    }
    str = String.fromCharCode.apply(null, str);
    print(str.length);

    var suffix1 = str.substr(str.length - 10, str.length);
    var suffix2 = '_' + suffix1;  // No match.

    for (i = 0; i < 1e6; i++) {
        void str.endsWith(suffix1);
        void str.endsWith(suffix2);
        void str.endsWith(suffix1);
        void str.endsWith(suffix2);
        void str.endsWith(suffix1);
        void str.endsWith(suffix2);
        void str.endsWith(suffix1);
        void str.endsWith(suffix2);
        void str.endsWith(suffix1);
        void str.endsWith(suffix2);
    }
}

test();
