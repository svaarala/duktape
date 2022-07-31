/*
 *  Proxy 'get' trap performance.
 */

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

test();
