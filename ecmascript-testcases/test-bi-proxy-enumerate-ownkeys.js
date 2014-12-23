/*
 *  Proxy (ES6 draft) 'enumerate' and 'ownKeys'.
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

    // Enumerate on proxy object triggers 'enumerate' trap.
    print('enum keys:', keys.join(' '));

    // Object.keys() on proxy object triggers 'ownKeys' trap.
    // If no such trap exists, enumerable property names of the *proxy*
    // object are listed (not the proxy target).
    try {
        print('Object.keys:', Duktape.enc('jx', Object.keys(obj)));
    } catch (e) {
        print('Object.keys:', e.name);
    }

    // Object.getOwnPropertyNames() on proxy object also triggers 'ownKeys'
    // trap.  If no such trap exists, property names (enumerable or not)
    // of the *proxy* object are listed (not the proxy target).
    try {
        print('Object.getOwnPropertyNames:', Duktape.enc('jx', Object.getOwnPropertyNames(obj)));
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
enumerate: true true
enum keys: string:foo string:bar string:quux string:nonEnum
Object.keys: ["foo","bar","quux"]
Object.getOwnPropertyNames: ["foo","bar","quux","nonEnum"]
enumerate: true true
enum keys: string:nosuch1 string:nosuch2
Object.keys: ["foo","bar","quux"]
Object.getOwnPropertyNames: ["foo","bar","quux"]
enum keys: string:foo string:bar string:quux string:enumProp
ownKeys: true true
Object.keys: ["foo","enumProp","nonEnumProp"]
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

    // Proxy which returns Object.getOwnPropertyNames() from 'enumerate' trap.
    // This causes enumeration to enumerate 'through' to the target object,
    // except that also non-enumerable properties get enumerated.
    // Object.keys() and Object.getOwnPropertyNames() operate directly on
    // the proxy target.

    target = { foo: 1, bar: 2, quux: [ 1, 2, 3 ] };
    Object.defineProperty(target, 'nonEnum', {
        value: 'nonenumerable', writable: true, enumerable: false, configurable: true
    });
    handler = {
        enumerate: function (targ) {
            print('enumerate:', this === handler, targ === target);
            return Object.getOwnPropertyNames(targ);
        }
    };
    proxy = new Proxy(target, handler);
    objDump(proxy);

    // Proxy which fabricates non-existent keys in 'enumerate'.
    // Object.keys() and Object.getOwnPropertyNames()
    // operate on the proxy target and see the actual keys.

    target = { foo: 1, bar: 2, quux: [ 1, 2, 3 ] };
    handler = {
        enumerate: function (targ) {
            print('enumerate:', this === handler, targ === target);
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
    // NOTE: ES6 draft algorithm for Object.keys() is that the trap
    // result list is processed to check whether or not each key is
    // enumerable (the check uses [[GetOwnProperty]]).  Duktape omits
    // this step now, so Object.keys() and Object.getOwnPropertyNames()
    // returns the same result for now.  However, the trap result will
    // still be cleaned up so that non-string keys, gaps, etc will be
    // eliminated.

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

print('basic test');

try {
    basicEnumerationTest();
} catch (e) {
    print(e);
}

/*===
trap result test
fake trap result: undefined
enum keys: TypeError
Object.keys: TypeError
Object.getOwnPropertyNames: TypeError
fake trap result: null
enum keys: TypeError
Object.keys: TypeError
Object.getOwnPropertyNames: TypeError
fake trap result: true
enum keys: TypeError
Object.keys: TypeError
Object.getOwnPropertyNames: TypeError
fake trap result: false
enum keys: TypeError
Object.keys: TypeError
Object.getOwnPropertyNames: TypeError
fake trap result: 123
enum keys: TypeError
Object.keys: TypeError
Object.getOwnPropertyNames: TypeError
fake trap result: "foo"
enum keys: TypeError
Object.keys: TypeError
Object.getOwnPropertyNames: TypeError
fake trap result: ["foo","bar","quux"]
enum keys: string:foo string:bar string:quux
Object.keys: ["foo","bar","quux"]
Object.getOwnPropertyNames: ["foo","bar","quux"]
fake trap result: {"0":"foo","1":"bar","2":"quux","3":"baz",length:3}
enum keys: string:foo string:bar string:quux
Object.keys: ["foo","bar","quux"]
Object.getOwnPropertyNames: ["foo","bar","quux"]
fake trap result: ["foo",undefined,null,true,false,123,"quux"]
enum keys: string:foo string:quux
Object.keys: ["foo","quux"]
Object.getOwnPropertyNames: ["foo","quux"]
fake trap result: {"0":"foo","1":123,"3":"quux",length:5}
enum keys: string:foo string:quux
Object.keys: ["foo","quux"]
Object.getOwnPropertyNames: ["foo","quux"]
fake trap result: ["foo-gappy",undefined,undefined,undefined,undefined,undefined,undefined,undefined,undefined,undefined,"bar-gappy",undefined,undefined,undefined,undefined,undefined,undefined,undefined,undefined,undefined,"quux-gappy",undefined,undefined,undefined,undefined,undefined,undefined,undefined,undefined,undefined]
enum keys: string:foo-gappy string:bar-gappy string:quux-gappy
Object.keys: ["foo-gappy","bar-gappy","quux-gappy"]
Object.getOwnPropertyNames: ["foo-gappy","bar-gappy","quux-gappy"]
fake trap result: {"0":"foo","1":"bar","2":"quux"}
enum keys: 
Object.keys: []
Object.getOwnPropertyNames: []
===*/

/* Check handling of various trap result checks: return value must be an object
 * but not necessarily an array, handling of gaps, handling of non-string values,
 * handling of missing 'length', etc.
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

    results.forEach(function (trapResult) {
        var target = {
        };
        var proxy;

        print('fake trap result:', Duktape.enc('jx', trapResult))

        proxy = new Proxy(target, {
            enumerate: function (targ) { return trapResult; },
            ownKeys: function (targ) { return trapResult; }
        });

        objDump(proxy);
    });
}

print('trap result test');

try {
    trapResultTest();
} catch (e) {
    print(e);
}
