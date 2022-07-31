/*
 *  Test duk_hstring arridx parsing.
 */

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

test();
