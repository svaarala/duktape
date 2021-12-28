if (typeof print !== 'function') { print = console.log; }

function test() {
    var str = [];
    var i;

    for (i = 0; i < 65536; i++) {
        str.push(i);
    }
    str = String.fromCharCode.apply(null, str);
    print(str.length);

    var strlen = str.length;
    for (i = 0; i < 2e3; i++) {
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
