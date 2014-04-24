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
counts: get=0 set=0 del=0
handler.get: true true string foo true
get foo: fake-for-key-foo
handler.get: true true string bar true
get bar: fake-for-key-bar
handler.get: true true string 1000 true
get 1000 (string): fake-for-key-1000
handler.get: true true number 1000 true
get 1000 (number): 2000
counts: get=4 set=0 del=0
handler.get: true true string special true
get special: specialValue
handler.get: true true string special true
get special: uncaughtValue
target.special: uncaughtValue
counts: get=6 set=0 del=0
handler.set: true true string foo number 123 true
handler.set: true true string bar number 234 true
handler.set: true true string quux number 345 true
handler.set: true true number 123 string foo true
handler.set: true true string rejectSet1 string reject true
handler.set: true true string rejectSet2 string reject true
counts: get=6 set=6 del=0
target.foo: 123
target.bar: 234
target.quux: 345
target[123]: foo
target.rejectSet1: undefined
target.rejectSet2: undefined
counts: get=6 set=6 del=0
target.foo: undefined
target.bar: 234
counts: get=6 set=6 del=0
handler.deleteProperty: true true string rejectSet1
handler.deleteProperty: true true string rejectSet2
handler.deleteProperty: true true string foo
handler.deleteProperty: true true number 1234
counts: get=6 set=6 del=4
target.rejectSet1: reject1
target.rejectSet2: reject2
target.foo undefined
target[1234] undefined
===*/

function subsetProxyTest() {
    var getCount = 0;
    var setCount = 0;
    var deleteCount = 0;
    var target = { foo: 123, '1000': 'thousand', special: 'specialValue' };
    var handler = {};
    var proxy = new Proxy(target, handler);

    function printCounts() {
        print('counts:', 'get=' + getCount, 'set=' + setCount, 'del=' + deleteCount);
    }

    print(typeof proxy);
    print(Object.prototype.toString.call(proxy));  // XXX: now class is 'Object'

    // handler 'get' hook
    handler.get = function(targ, key, receiver) {
        print('handler.get:', this === handler, targ === target, typeof key, key, receiver === proxy);
        getCount++;
        if (typeof key === 'number') {
            return 2 * (+key);
        } else if (key === 'special') {
            return targ.special;
        } else {
            return 'fake-for-key-' + key;
        }
    };

    // Get tests
    printCounts();
    print('get foo:', proxy.foo);
    print('get bar:', proxy.bar);
    print('get 1000 (string):', proxy['1000']);
    print('get 1000 (number):', proxy[1000]);
    printCounts();

    // without a 'set' hook, writes go through
    print('get special:', proxy.special);
    proxy.special = 'uncaughtValue';  // goes into 'target'
    print('get special:', proxy.special);
    print('target.special:', target.special);

    // handler 'set' hook
    handler.set = function(targ, key, val, receiver) {
        print('handler.set:', this === handler, targ === target, typeof key, key, typeof val, val, receiver === proxy);
        setCount++;
        if (key === 'rejectSet1') {
            // false return code prevents target object from being modified
            return false;
        }
        if (key === 'rejectSet2') {
            // same for any 'falsy' value
            return 0;
        }
        return true;
    };

    // Set tests
    printCounts();
    proxy.foo = 123;
    proxy.bar = 234;
    proxy.quux = 345;
    proxy[123] = 'foo';
    proxy.rejectSet1 = 'reject';
    proxy.rejectSet2 = 'reject';
    printCounts();
    print('target.foo:', target.foo);
    print('target.bar:', target.bar);
    print('target.quux:', target.quux);
    print('target[123]:', target[123]);
    print('target.rejectSet1:', target.rejectSet1);
    print('target.rejectSet2:', target.rejectSet2);

    // without a 'deleteProperty' hook, deletes go through
    printCounts();
    delete proxy.foo;
    print('target.foo:', target.foo);
    print('target.bar:', target.bar);
    printCounts();

    // handler 'deleteProperty' hook
    handler.deleteProperty = function(targ, key) {
        print('handler.deleteProperty:', this === handler, targ === target, typeof key, key);
        deleteCount++;
        if (key === 'rejectSet1') {
            // false return code prevents target object from being modified
            return false;
        }
        if (key === 'rejectSet2') {
            // same for any 'falsy' value
            return 0;
        }
        return true;
    };

    target.rejectSet1 = 'reject1';
    target.rejectSet2 = 'reject2';
    target.foo = 123;
    target[1234] = 4321;
    delete proxy.rejectSet1;
    delete proxy.rejectSet2;
    delete proxy.foo;  // allowed
    delete proxy[1234];  // allowed
    printCounts();
    print('target.rejectSet1:', target.rejectSet1);
    print('target.rejectSet2:', target.rejectSet2);
    print('target.foo', target.foo);
    print('target[1234]', target[1234]);
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

    // FIXME: unimplemented
}

print('proxy revocation');

try {
    proxyRevocationTest();
} catch (e) {
    print(e.name);
}
