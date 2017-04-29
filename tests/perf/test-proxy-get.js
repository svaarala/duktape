/*
 *  Proxy 'get' trap performance.
 */

if (typeof print !== 'function') { print = console.log; }

function test() {
    var i;
    var proxy = new Proxy({}, {
        get: function () {
            return 123;
        }
    });

    for (i = 0; i < 1e5; i++) {
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
        void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo; void proxy.foo;
    }
}
try {
    test();
} catch (e) {
    print(e.stack || e);
    throw e;
}
