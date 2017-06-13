if (typeof print !== 'function') { print = console.log; }

function test() {
    var clz32 = Math.clz32;
    var i;

    for (i = 0; i < 1e6; i++) {
        void clz32(0x80000000);
        void clz32(0x40000000);
        void clz32(0x20000000);
        void clz32(0x10000000);

        void clz32(0x08000000);
        void clz32(0x04000000);
        void clz32(0x02000000);
        void clz32(0x01000000);

        void clz32(0x00800000);
        void clz32(0x00400000);
        void clz32(0x00200000);
        void clz32(0x00100000);

        void clz32(0x00080000);
        void clz32(0x00040000);
        void clz32(0x00020000);
        void clz32(0x00010000);

        void clz32(0x00008000);
        void clz32(0x00004000);
        void clz32(0x00002000);
        void clz32(0x00001000);

        void clz32(0x00000800);
        void clz32(0x00000400);
        void clz32(0x00000200);
        void clz32(0x00000100);

        void clz32(0x00000080);
        void clz32(0x00000040);
        void clz32(0x00000020);
        void clz32(0x00000010);

        void clz32(0x00000008);
        void clz32(0x00000004);
        void clz32(0x00000002);
        void clz32(0x00000001);

        void clz32(0x00000000);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
