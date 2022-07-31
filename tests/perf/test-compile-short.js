function test() {
    var i;
    var src = '123 + myval';
    var myval = 234;

    for (i = 0; i < 1e5; i++) {
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
        void eval(src);
    }
}

test();
