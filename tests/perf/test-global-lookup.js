/*
 *  Global variable lookup.  This matters because all calls like
 *  "print('hello');" go through a slow path GETVAR lookup.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;

    for (i = 0;i < 1e6; i++) {
        /* Repeat 100x to ensure loop control overhead is negligible. */

        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;

        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;

        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;

        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;

        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;

        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;

        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;

        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;

        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;

        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
        void print;
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
