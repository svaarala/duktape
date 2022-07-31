function test() {
    var str = [];
    var i;

    for (i = 0; i < 1024; i++) {
        str.push(i);
    }
    str = String.fromCharCode.apply(null, str);
    print(str.length);

    var strlen = str.length;
    for (i = 0; i < 1e5; i++) {
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
