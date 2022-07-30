/*
 *  Global variable lookup.  This matters because all calls like
 *  "print('hello');" go through a slow path GETVAR lookup.
 */

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

test();
