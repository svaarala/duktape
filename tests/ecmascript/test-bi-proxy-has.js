/*
 *  Proxy (ES6) 'has'.
 */

function propDump(obj, prop) {
    try {
        print(prop + ': ' + (prop in obj));
    } catch (e) {
        print(prop + ': ' + e.name);
    }
}

/*===
basic test
foo: true
1: true
4: false
foo: true
baz: false
handler.has: true true string 0
0: true
handler.has: true true string 1
1: true
handler.has: true true string 2
2: true
handler.has: true true string 3
3: false
handler.has: true true string 0
0: true
handler.has: true true string 1
1: true
handler.has: true true string 2
2: true
handler.has: true true string 3
3: false
handler.has: true true string key1
key1: false
handler.has: true true string key2
key2: true
handler.has: true true string key3
key3: false
handler.has: true true string key4
key4: true
handler.has: true true string key5
key5: false
handler.has: true true string key6
key6: true
handler.has: true true string foo
foo: false
handler.has: true true string baz
baz: true
handler.has: true true string foo
foo: true
handler.has: true true string bar
bar: true
handler.has: true true string quux
quux: true
handler.has: true true string foo
foo: true
handler.has: true true string bar
bar: true
handler.has: true true string quux
quux: true
handler.has: true true string foo
foo: false
handler.has: true true string bar
bar: TypeError
handler.has: true true string quux
quux: false
handler.has: true true string foo
foo: TypeError
handler.has: true true string bar
bar: TypeError
handler.has: true true string quux
quux: false
===*/

function basicExistenceTest() {
    var target;
    var proxy;
    var handler;

    // Simple target object, no proxies.

    target = { foo: 1, bar: 2, quux: [ 1, 2, 3 ] };
    propDump(target, 'foo');
    target = [ 'foo', 'bar', 'quux' ];
    target.prop = 'propval';
    propDump(target, '1');
    propDump(target, '4');

    // Simple target object, proxy without 'has'.  Existence check
    // goes through to the proxy target.

    target = { foo: 1, bar: 2, quux: [ 1, 2, 3 ] };
    handler = {};
    proxy = new Proxy(target, handler);
    propDump(proxy, 'foo');
    propDump(proxy, 'baz');

    // Proxy target, check key coercion.
    // For 'x in y', 'x' is string coerced before HasProperty() internal
    // algorithm is invoked so in ES6 the key is always a string (!).

    target = [ 'foo', 'bar', 'quux' ];
    handler = {
        has: function (targ, key) {
            print('handler.has:', this === handler, targ === target, typeof key, key);
            return key in targ;
        }
    };
    proxy = new Proxy(target, handler);
    propDump(proxy, '0');
    propDump(proxy, '1');
    propDump(proxy, '2');
    propDump(proxy, '3');
    propDump(proxy, 0);
    propDump(proxy, 1);
    propDump(proxy, 2);
    propDump(proxy, 3);

    // Proxy return value is boolean coerced.

    target = {};
    handler = {
        has: function (targ, key) {
            print('handler.has:', this === handler, targ === target, typeof key, key);
            if (key === 'key1') { return 0; }
            if (key === 'key2') { return 1; }
            if (key === 'key3') { return false; }
            if (key === 'key4') { return true; }
            if (key === 'key5') { return ''; }
            if (key === 'key6') { return 'foo'; }
            return false;
        }
    };
    proxy = new Proxy(target, handler);
    propDump(proxy, 'key1');
    propDump(proxy, 'key2');
    propDump(proxy, 'key3');
    propDump(proxy, 'key4');
    propDump(proxy, 'key5');
    propDump(proxy, 'key6');

    // Proxies which return true/false regardless of what is in the target.
    // The target is extensible so this doesn't matter.

    target = { foo: 1, bar: 2, quux: [ 1, 2, 3 ] };
    handler = {
        has: function (targ, key) {
            print('handler.has:', this === handler, targ === target, typeof key, key);
            if (key === 'foo') { return false; }  // false when key exists
            if (key === 'baz') { return true; }   // true when key does not exist
            return true;
        }
    };
    proxy = new Proxy(target, handler);
    propDump(proxy, 'foo');
    propDump(proxy, 'baz');

    // Proxy indicates property exists.  This is never rejected, even if
    // the target object is non-extensible.

    target = { foo: 1 };
    Object.defineProperty(target, 'bar', {
        value: 2, writable: true, enumerable: true, configurable: false
    });
    handler = {
        has: function (targ, key) {
            print('handler.has:', this === handler, targ === target, typeof key, key);
            return true;
        }
    };
    proxy = new Proxy(target, handler);
    propDump(proxy, 'foo');
    propDump(proxy, 'bar');
    propDump(proxy, 'quux');

    Object.preventExtensions(target);
    propDump(proxy, 'foo');
    propDump(proxy, 'bar');
    propDump(proxy, 'quux');

    // Proxy indicates property does not exist.  This is rejected in some cases.

    target = { foo: 1 };
    Object.defineProperty(target, 'bar', {
        value: 2, writable: true, enumerable: true, configurable: false
    });
    handler = {
        has: function (targ, key) {
            print('handler.has:', this === handler, targ === target, typeof key, key);
            return false;
        }
    };
    proxy = new Proxy(target, handler);
    propDump(proxy, 'foo');
    propDump(proxy, 'bar');  // rejected: exists in target, non-configurable
    propDump(proxy, 'quux');

    Object.preventExtensions(target);
    propDump(proxy, 'foo');   // rejected: exists in target, target not extensible
    propDump(proxy, 'bar');   // rejected: exists in target, non-configurable
    propDump(proxy, 'quux');  // accepted: does not exist in target, target not extensible; es6 rev23/24 still allows this
}

print('basic test');

try {
    basicExistenceTest();
} catch (e) {
    print(e);
}
