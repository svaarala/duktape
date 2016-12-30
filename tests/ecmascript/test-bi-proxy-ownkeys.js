/*
 *  Proxy (ES2015) 'ownKeys'.
 */

function objDump(obj) {
    var k;
    var keys = [];
    try {
        for (k in obj) {
            keys.push((k === null ? 'null' : typeof k) + ':' + String(k));
        }
    } catch (e) {
        keys = [ e.name ];
    }

    // Enumerate on proxy object triggers 'ownKeys' trap in ES2016
    // ('enumerate' trap in ES2015).
    print('enum keys:', keys.join(' '));

    // Object.keys() on proxy object triggers 'ownKeys' trap.
    // If no such trap exists, enumerable property names of the *proxy*
    // object are listed (not the proxy target).
    try {
        print('Object.keys:', JSON.stringify(Object.keys(obj)));
    } catch (e) {
        print('Object.keys:', e.name);
    }

    // Object.getOwnPropertyNames() on proxy object also triggers 'ownKeys'
    // trap.  If no such trap exists, property names (enumerable or not)
    // of the *proxy* object are listed (not the proxy target).
    try {
        print('Object.getOwnPropertyNames:', JSON.stringify(Object.getOwnPropertyNames(obj)));
    } catch (e) {
        print('Object.getOwnPropertyNames:', e.name);
    }
}

/*===
basic test
enum keys: string:foo string:bar string:quux
Object.keys: ["foo","bar","quux"]
Object.getOwnPropertyNames: ["foo","bar","quux"]
enum keys: string:0 string:1 string:2 string:prop
Object.keys: ["0","1","2","prop"]
Object.getOwnPropertyNames: ["0","1","2","length","prop"]
proxy.foo: 1
enum keys: string:foo string:bar string:quux
Object.keys: ["foo","bar","quux"]
Object.getOwnPropertyNames: ["foo","bar","quux"]
enum keys: string:foo string:bar string:quux
Object.keys: ["foo","bar","quux"]
Object.getOwnPropertyNames: ["foo","bar","quux","nonEnum"]
enum keys: string:foo string:bar string:quux
Object.keys: ["foo","bar","quux"]
Object.getOwnPropertyNames: ["foo","bar","quux"]
ownKeys: true true
enum keys: string:foo string:enumProp
ownKeys: true true
Object.keys: ["foo","enumProp"]
ownKeys: true true
Object.getOwnPropertyNames: ["foo","enumProp","nonEnumProp"]
===*/

function basicEnumerationTest() {
    var target;
    var proxy;
    var handler;

    // Simple target object, no proxies.

    target = { foo: 1, bar: 2, quux: [ 1, 2, 3 ] };
    objDump(target);
    target = [ 'foo', 'bar', 'quux' ];
    target.prop = 'propval';
    objDump(target);

    // Simple target object, proxy without 'enumerate' or 'ownKeys'.
    // Enumeration goes through to the proxy target, same applies to
    // Object.keys() and Object.getOwnPropertyNames().

    target = { foo: 1, bar: 2, quux: [ 1, 2, 3 ] };
    proxy = new Proxy(target, {
    });
    print('proxy.foo:', proxy.foo);
    objDump(proxy);

    // Proxy which returns Object.getOwnPropertyNames() from 'getKeys' trap.
    // This causes enumeration to enumerate 'through' to the target object,
    // except that also non-enumerable properties get enumerated.

    target = { foo: 1, bar: 2, quux: [ 1, 2, 3 ] };
    Object.defineProperty(target, 'nonEnum', {
        value: 'nonenumerable', writable: true, enumerable: false, configurable: true
    });
    handler = {
        getKeys: function (targ) {
            print('getKeys:', this === handler, targ === target);
            return Object.getOwnPropertyNames(targ);
        }
    };
    proxy = new Proxy(target, handler);
    objDump(proxy);

    // Proxy which fabricates non-existent keys in 'getKeys'.

    target = { foo: 1, bar: 2, quux: [ 1, 2, 3 ] };
    handler = {
        getKeys: function (targ) {
            print('getKeys:', this === handler, targ === target);
            return [ 'nosuch1', 'nosuch2' ];
        }
    };
    proxy = new Proxy(target, handler);
    objDump(proxy);

    // Proxy which provides a subset of keys in 'ownKeys'.
    // Enumeration has no trap so it operates on the target directly
    // (and only sees enumerable properties).  Object.keys() and
    // Object.getOwnPropertyNames() use the 'ownKeys' trap.
    //
    // NOTE: ES2015 algorithm for Object.keys() is that the trap result list is
    // processed to check whether or not each key is enumerable (the check
    // uses [[GetOwnProperty]]).  Duktape omits this step now, so
    // Object.keys() and Object.getOwnPropertyNames() return the same result
    // for now.  However, the trap result will still be cleaned up so that
    // non-string keys, gaps, etc will be eliminated.

    target = { foo: 1, bar: 2, quux: [ 1, 2, 3 ] };
    Object.defineProperties(target, {
        enumProp: {
            value: 'enumerable', writable: true, enumerable: true, configurable: true,
        },
        nonEnumProp: {
            value: 'non-enumerable', writable: true, enumerable: false, configurable: true,
        }
    });
    handler = {
        ownKeys: function (targ) {
            print('ownKeys:', this === handler, targ === target);
            return [ 'foo', 'enumProp', 'nonEnumProp' ];
        }
    };
    proxy = new Proxy(target, handler);
    objDump(proxy);
}

try {
    print('basic test');
    basicEnumerationTest();
} catch (e) {
    print(e);
}

/*===
trap result test
fake trap result: 0
enum keys: TypeError
Object.keys: TypeError
Object.getOwnPropertyNames: TypeError
fake trap result: 1
enum keys: TypeError
Object.keys: TypeError
Object.getOwnPropertyNames: TypeError
fake trap result: 2
enum keys: TypeError
Object.keys: TypeError
Object.getOwnPropertyNames: TypeError
fake trap result: 3
enum keys: TypeError
Object.keys: TypeError
Object.getOwnPropertyNames: TypeError
fake trap result: 4
enum keys: TypeError
Object.keys: TypeError
Object.getOwnPropertyNames: TypeError
fake trap result: 5
enum keys: TypeError
Object.keys: TypeError
Object.getOwnPropertyNames: TypeError
fake trap result: 6
enum keys: 
Object.keys: []
Object.getOwnPropertyNames: ["foo","bar","quux"]
fake trap result: 7
enum keys: 
Object.keys: []
Object.getOwnPropertyNames: ["foo","bar","quux"]
fake trap result: 8
enum keys: TypeError
Object.keys: TypeError
Object.getOwnPropertyNames: TypeError
fake trap result: 9
enum keys: TypeError
Object.keys: TypeError
Object.getOwnPropertyNames: TypeError
fake trap result: 10
enum keys: TypeError
Object.keys: TypeError
Object.getOwnPropertyNames: TypeError
fake trap result: 11
enum keys: 
Object.keys: []
Object.getOwnPropertyNames: []
===*/

/* Check handling of various trap result checks: return value must be an object
 * but not necessarily an array.  In ES2016 gaps, non-string/symbol values, etc.
 * cause a TypeError.
 */

function trapResultTest() {
    var gappyArray = [];
    gappyArray[0] = 'foo-gappy';
    gappyArray[10] = 'bar-gappy';
    gappyArray[20] = 'quux-gappy';
    gappyArray.length = 30;
    var results = [
        undefined, null, true, false, 123, 'foo',                            // non-object -> TypeError
        [ 'foo', 'bar', 'quux' ],                                            // array
        { '0': 'foo', '1': 'bar', '2': 'quux', '3': 'baz', 'length': 3 },    // array-like object, 'baz' is skipped (over 'length')
        [ 'foo', undefined, null, true, false, 123, 'quux' ],                // non-string values
        { '0': 'foo', '1': 123, '3': 'quux', 'length': 5 },                  // non-string values and gaps, array-like object
        gappyArray,                                                          // array with gaps
        { '0': 'foo', '1': 'bar', '2': 'quux' },                             // object without 'length'
    ];

    results.forEach(function (trapResult, idx) {
        var target = {
        };
        var proxy;

        print('fake trap result:', idx);
        // print(Duktape.enc('jx', trapResult));

        proxy = new Proxy(target, {
            enumerate: function (targ) { print('"enumerate" trap called, not intended in ES2016'); return trapResult; },
            ownKeys: function (targ) { return trapResult; }
        });

        objDump(proxy);
    });
}

try {
    print('trap result test');
    trapResultTest();
} catch (e) {
    print(e.stack || e);
}

/*===
proxy in Object.keys() etc
foo
foo,bar
Symbol(quux),Symbol(baz)
foo,bar,Symbol(quux),Symbol(baz)
===*/

/* Proxy in Object.keys(), Object.getOwnPropertyNames(), etc. */

function proxyInKeysTest() {
    var proxy;
    var target = {};
    Object.defineProperties(target, {
        foo: { value: 'foo-value', enumerable: true },
        bar: { value: 'bar-value', enumerable: false },
        [ Symbol.for('quux') ]: { value: 'quux-value', enumerable: true },
        [ Symbol.for('baz') ]: { value: 'quux-value', enumerable: false }
    });
    var proxy = new Proxy(target, {
    });

    print(Object.keys(proxy).map(String));
    print(Object.getOwnPropertyNames(proxy).map(String));
    print(Object.getOwnPropertySymbols(proxy).map(String));
    print(Reflect.ownKeys(proxy).map(String));
}

try {
    print('proxy in Object.keys() etc');
    proxyInKeysTest();
} catch (e) {
    print(e.stack || e);
}
