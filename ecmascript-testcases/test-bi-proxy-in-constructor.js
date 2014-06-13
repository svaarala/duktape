/*
 *  If an Ecmascript constructor returns an object value, the value replaces
 *  the default instance created (the 'this' value that a constructor gets).
 *  This can be used to wrap the 'this' value behind a Proxy.
 */

/*===
BAR
===*/

function MyConstructor(x) {
    this.foo = x;
    return new Proxy(this, {
        get: function (targ, key, recv) {
            var val = targ[key];
            return (typeof val === 'string' ? val.toUpperCase() : val);
        }
    });
}

function test() {
    var o = new MyConstructor('bar');
    print(o.foo);
}

try {
    test();
} catch (e) {
    print(e);
}
