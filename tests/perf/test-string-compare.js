/*
 *  Test string comparison.  Interned strings compare by pointer so there's
 *  not much going on here.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var a, b, c, d, e, f, g;
    var i;

    function mk(n) {
        var res = [];
        for (var i = 0; i < n; i++) { res[i] = 'x'; }
        res = res.join('');
        print(res.length);
        return res;
    }

    a = mk(0);
    b = mk(1);
    c = mk(16);
    d = mk(256);
    e = mk(4096);
    f = mk(65536);
    g = mk(1048576);

    for (i = 0; i < 1e7; i++) {
        void (a == a);
        void (a == b);
        void (a == c);
        void (a == d);
        void (a == e);
        void (a == f);
        void (a == g);

        void (b == b);
        void (b == c);
        void (b == d);
        void (b == e);
        void (b == f);
        void (b == g);

        void (c == c);
        void (c == d);
        void (c == e);
        void (c == f);
        void (c == g);

        void (d == d);
        void (d == e);
        void (d == f);
        void (d == g);

        void (e == e);
        void (e == f);
        void (e == g);

        void (f == f);
        void (f == g);

        void (g == g);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
