if (typeof print !== 'function') { print = console.log; }

function test() {
    var str = [];
    var i;

    for (i = 0; i < 1024; i++) {
        str.push(i & 0x7f);
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

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
