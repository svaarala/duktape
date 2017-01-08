/*
 *  Test duk_hstring arridx parsing.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var tmp = '1234567';
    var ref = tmp + '8';  // keep reachable so that no string is actually interned

    // create string on-the-fly, intern check (= arridx)
    for (i = 0; i < 1e6; i++) {
        void (tmp + '8');
        void (tmp + '8');
        void (tmp + '8');
        void (tmp + '8');
        void (tmp + '8');
        void (tmp + '8');
        void (tmp + '8');
        void (tmp + '8');
        void (tmp + '8');
        void (tmp + '8');
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
