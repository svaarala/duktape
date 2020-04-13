if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var tmp = 'foobarquux';

    for (i = 0; i < 1e5; i++) {
        void tmp[0]; void tmp[1]; void tmp[2]; void tmp[3]; void tmp[4];
        void tmp[5]; void tmp[6]; void tmp[7]; void tmp[8]; void tmp[9];

        void tmp[0]; void tmp[1]; void tmp[2]; void tmp[3]; void tmp[4];
        void tmp[5]; void tmp[6]; void tmp[7]; void tmp[8]; void tmp[9];

        void tmp[0]; void tmp[1]; void tmp[2]; void tmp[3]; void tmp[4];
        void tmp[5]; void tmp[6]; void tmp[7]; void tmp[8]; void tmp[9];

        void tmp[0]; void tmp[1]; void tmp[2]; void tmp[3]; void tmp[4];
        void tmp[5]; void tmp[6]; void tmp[7]; void tmp[8]; void tmp[9];

        void tmp[0]; void tmp[1]; void tmp[2]; void tmp[3]; void tmp[4];
        void tmp[5]; void tmp[6]; void tmp[7]; void tmp[8]; void tmp[9];

        void tmp[0]; void tmp[1]; void tmp[2]; void tmp[3]; void tmp[4];
        void tmp[5]; void tmp[6]; void tmp[7]; void tmp[8]; void tmp[9];

        void tmp[0]; void tmp[1]; void tmp[2]; void tmp[3]; void tmp[4];
        void tmp[5]; void tmp[6]; void tmp[7]; void tmp[8]; void tmp[9];

        void tmp[0]; void tmp[1]; void tmp[2]; void tmp[3]; void tmp[4];
        void tmp[5]; void tmp[6]; void tmp[7]; void tmp[8]; void tmp[9];

        void tmp[0]; void tmp[1]; void tmp[2]; void tmp[3]; void tmp[4];
        void tmp[5]; void tmp[6]; void tmp[7]; void tmp[8]; void tmp[9];

        void tmp[0]; void tmp[1]; void tmp[2]; void tmp[3]; void tmp[4];
        void tmp[5]; void tmp[6]; void tmp[7]; void tmp[8]; void tmp[9];
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
