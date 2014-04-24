/*
 *  Proxy (ES6 draft)
 */

/*===
proxy existence
Proxy exists: true function
Proxy.length: 2
Proxy.name: Proxy
Proxy desc: writable=true enumerable=false configurable=true
Proxy.revocable exists: true function
Proxy.revocable.length: 2
Proxy.revocable.name: revocable
Proxy.revocable desc: writable=true enumerable=false configurable=true
===*/

function proxyExistenceTest() {
    var pd;

    print('Proxy exists:', 'Proxy' in this, typeof this.Proxy);
    print('Proxy.length:', this.Proxy.length);
    print('Proxy.name:', this.Proxy.name);

    pd = Object.getOwnPropertyDescriptor(this, 'Proxy');
    if (pd) {
        print('Proxy desc:', 'writable=' + pd.writable, 'enumerable=' + pd.enumerable,
              'configurable=' + pd.configurable);
    }

    print('Proxy.revocable exists:', 'revocable' in this.Proxy, typeof this.Proxy.revocable);
    print('Proxy.revocable.length:', this.Proxy.revocable.length);
    print('Proxy.revocable.name:', this.Proxy.revocable.name);

    pd = Object.getOwnPropertyDescriptor(this.Proxy, 'revocable');
    if (pd) {
        print('Proxy.revocable desc:', 'writable=' + pd.writable, 'enumerable=' + pd.enumerable,
              'configurable=' + pd.configurable);
    }


    // FIXME: lengths and names
}

print('proxy existence');

try {
    proxyExistenceTest();
} catch (e) {
    print(e);
}

/*===
subset proxy
object
[object Object]
handler.get: true true string foo true
get foo: fake-for-key-foo
handler.get: true true string bar true
get bar: fake-for-key-bar
handler.get: true true string 1000 true
get 1000 (string): fake-for-key-1000
handler.get: true true number 1000 true
get 1000 (number): 2000
===*/

function subsetProxyTest() {
    var target = { foo: 123, '1000': 'thousand' };
    var handler = {
        get: function(targ, key, receiver) {
            print('handler.get:', this === handler, targ === target, typeof key, key, receiver === proxy);
            if (typeof key === 'number') {
                return 2 * (+key);
            } else {
                return 'fake-for-key-' + key;
            }
        }
    };
    var proxy = new Proxy(target, handler);

    print(typeof proxy);
    print(Object.prototype.toString.call(proxy));  // XXX: now class is 'Object'

    print('get foo:', proxy.foo);
    print('get bar:', proxy.bar);
    print('get 1000 (string):', proxy['1000']);
    print('get 1000 (number):', proxy[1000]);
}

print('subset proxy');

try {
    subsetProxyTest();
} catch (e) {
    print(e);
}

/*===
recursive proxies
TypeError
TypeError
===*/

/* Currently Duktape doesn't allow a proxy as either a handler or a target.
 * This makes it easier to implement because there is no arbitrary depth C
 * recursion when doing proxy lookups.
 */

function proxyHandlerTest() {
    var target = { foo: 123 };
    var handler = new Proxy({}, {});
    var proxy = new Proxy(target, handler);
}

function proxyTargetTest() {
    var target = new Proxy({}, {});
    var handler = {};
    var proxy = new Proxy(target, handler);
}

print('recursive proxies');

try {
    proxyHandlerTest();
} catch (e) {
    print(e.name);
}

try {
    proxyTargetTest();
} catch (e) {
    print(e.name);
}

/*===
getter handler
handler.get: true true foo true
proxy.foo: dummy-value
===*/

/* A getter as a handler property.  No reason why this wouldn't work but
 * test just in case.
 */

function getterHandlerTest() {
    var target = { foo: 123 };
    var handler = {};
    var proxy = new Proxy(target, handler);

    Object.defineProperty(handler, 'get', {
        enumerable: true,
        configurable: true,
        get: function () {
            return function (targ, key, receiver) {
                print('handler.get:', this === handler, targ === target, key, receiver === proxy);
                return 'dummy-value';
            };
        }
    });

    print('proxy.foo:', proxy.foo);
}

print('getter handler');

try {
    getterHandlerTest();
} catch (e) {
    print(e);
}

/*===
non-callable handler
TypeError
===*/

/* A non-callable handler property.  This is not checked during proxy creation
 * and should cause a TypeError.
 */

function nonCallableHandlerTest() {
    var target = { foo: 123 };
    var handler = { get: 'dummy' };
    var proxy = new Proxy(target, handler);
    print('proxy.foo:', proxy.foo);
}

print('non-callable handler');

try {
    nonCallableHandlerTest();
} catch (e) {
    print(e.name);
}

/*===
throwing handler
handler.get: about to throw
URIError: fake error
===*/

/* Handler function throws.  Nothing special here. */

function throwingHandlerTest() {
    var target = { foo: 123 };
    var handler = {
        get: function() {
            print('handler.get: about to throw');
            throw new URIError('fake error');
        }
    };
    var proxy = new Proxy(target, handler);
    print('proxy.foo:', proxy.foo);

}

print('throwing handler');

try {
    throwingHandlerTest();
} catch (e) {
    print(e);
}

/*===
proxy revocation
handler.get: true true foo true
proxy.foo: dummy-value
===*/

/* Revoked proxy. */

function proxyRevocationTest() {
    var target = { foo: 123 };
    var handler = {
        get: function(targ, key, receiver) {
            print('handler.get:', this === handler, targ === target, key, receiver === proxy);
            return 'dummy-value';
        }
    };
    var proxy = new Proxy(target, handler);
    print('proxy.foo:', proxy.foo);

    print('FIXME, unimplemented');
}

print('proxy revocation');

try {
    proxyRevocationTest();
} catch (e) {
    print(e.name);
}
