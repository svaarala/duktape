function test() {
    var str = [];
    var i;

    for (i = 0; i < 65536; i++) {
        str.push(i);
    }
    str = String.fromCharCode.apply(null, str).repeat(16);  // 1M codepoints
    print(str.length);

    var strlen = str.length;
    for (i = 0; i < 2e2; i++) {
        void str.substring(Math.random() * strlen, Math.random() * strlen);
        void str.substring(Math.random() * strlen, Math.random() * strlen);
        void str.substring(Math.random() * strlen, Math.random() * strlen);
        void str.substring(Math.random() * strlen, Math.random() * strlen);
        void str.substring(Math.random() * strlen, Math.random() * strlen);
        void str.substring(Math.random() * strlen, Math.random() * strlen);
        void str.substring(Math.random() * strlen, Math.random() * strlen);
        void str.substring(Math.random() * strlen, Math.random() * strlen);
        void str.substring(Math.random() * strlen, Math.random() * strlen);
        void str.substring(Math.random() * strlen, Math.random() * strlen);
    }
}

test();
