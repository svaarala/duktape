/*@include util-buffer.js@*/

/*===
proxy test
Proxy get for key: BYTES_PER_ELEMENT
1
Proxy get for key: 0
97
Proxy get for key: subarray
function
123
function
get
this: object false [object Object]
target: object false [object Object]
key: string foo
proxy.foo: bar
get
this: object false [object Object]
target: object false [object Object]
key: string nonExistent
proxy.nonExistent: dummy
===*/

function proxyTest() {
    var pb;
    var proxy;

    // Plain buffer is accepted as a proxy target, but object coerced.
    pb = createPlainBuffer('abcdefghijklmnop');
    proxy = new Proxy(pb, {
        get: function get(targ, key, receiver) {
            print('Proxy get for key:', key);
            return targ[key];
        }
    });
    print(proxy.BYTES_PER_ELEMENT);
    print(proxy[0]);
    print(typeof proxy.subarray);
    proxy[0] = 123;
    print(pb[0]);

    // Proxy as a handler value; ES2015 requires it must be an Object and a
    // plain buffer pretends to be an object.  The traps must be placed in
    // Uint8Array.prototype for it to actually work - so this is not a very
    // useful thing in practice.  Currently Proxy will just coerce the plain
    // buffer to a full Uint8Array silently.

    Uint8Array.prototype.get = function (target, key) {
        print('get');
        print('this:', typeof this, isPlainBuffer(target), target);
        print('target:', typeof target, isPlainBuffer(target), target);
        print('key:', typeof key, key);
        return target[key] || 'dummy';  // passthrough
    };

    pb = createPlainBuffer('abcdefghijklmnop');
    print(typeof pb.get);
    proxy = new Proxy({ foo: 'bar' }, pb);
    print('proxy.foo:', proxy.foo);
    print('proxy.nonExistent:', proxy.nonExistent);

    delete Uint8Array.prototype.get;
}

try {
    print('proxy test');
    proxyTest();
} catch (e) {
    print(e.stack || e);
}
