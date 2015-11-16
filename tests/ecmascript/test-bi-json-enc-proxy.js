/*
 *  Stringifying a Proxy should consult the underlying object.
 */

/*===
{"foo":234}
{"foo":345}
===*/

/* Minimal case where Proxy has no traps that interfere with JSON
 * stringification.
 */

function jsonStringifyNakedProxyTest() {
    var target, proxy, obj;

    // Here keys to be enumerated are from the target object.
    // (Verified to behave the same way with Firefox.)
    target = { foo: 123 };
    proxy = new Proxy(target, { get: function () { return 234; } });
    print(JSON.stringify(proxy));

    // Inherited properties are not enumerated.
    // (Verified to behave the same way with Firefox.)
    obj = { bar: 234 };  // not serialized
    target = Object.create(obj);
    target.foo = 123;  // serialized
    proxy = new Proxy(target, { get: function () { return 345; } });
    print(JSON.stringify(proxy));
}

try {
    jsonStringifyNakedProxyTest();
} catch (e) {
    print(e.stack || e);
}

/*===
ownKeys should be called
{"foo":"key-foo","quux":"key-quux"}
ownKeys should be called
getOwnPropertyDescriptor for foo
getOwnPropertyDescriptor for quux
getOwnPropertyDescriptor for baz
getOwnPropertyDescriptor for nonEnumerable
{"foo":"key-foo","quux":"key-quux","baz":"key-baz"}
===*/

/* Proxy 'ownKeys' trap is consulted, 'enumerate' trap is NOT consulted.
 * This has been verified to match Firefox behavior.
 */

function jsonStringifyOwnKeysProxyTest() {
    var target, proxy, obj;

    // 'keys' trap is invoked to figure out keys to serialize
    // - JSON.stringify() calls internal function: http://www.ecma-international.org/ecma-262/6.0/#sec-serializejsonobject
    // - SerializeJSONObject() calls internal function: http://www.ecma-international.org/ecma-262/6.0/#sec-enumerableownnames
    // - EnumerableOwnNames() calls O.[[OwnPropertyKeys]]
    // - Proxy [[OwnPropertyKeys]] calls 'ownKeys' trap: http://www.ecma-international.org/ecma-262/6.0/#sec-proxy-object-internal-methods-and-internal-slots-ownpropertykeys
    // - Assuming 'ownKeys' trap result validation succeeds, EnumerableOwnNames() filters out non-enumerable target properties;
    //   the property descriptor is obtained using [[GetOwnProperty]] which may invoke further traps
    // - EnumerableOwnNames() reorders the resulting keys to match [[Enumerate]] order
    //   (not sure what to do with a trap result?)

    target = { foo: 123, bar: 234, quux: 345, nonEnumerable: 456 };
    Object.defineProperty(target, 'nonEnumerable', { enumerable: false });
    proxy = new Proxy(target, {
        get: function (targ, key, recv) {
            return 'key-' + key;
        },
        enumerate: function () {
            print('enumerate should not be called');
            return [ 'quux' ];
        },
        ownKeys: function () {
            // ownKeys is called to decide what properties to serialize.
            // Non-enumerable properties are filtered out from JSON result.
            print('ownKeys should be called');
            return [ 'foo', 'quux', 'baz', 'nonEnumerable' ];
        }
    });
    print(JSON.stringify(proxy));

    // With 'getOwnPropertyDescriptor' present that gets consulted to figure
    // out what 'ownKeys' result keys are enumerable.

    target = { foo: 123, bar: 234, quux: 345, nonEnumerable: 456 };
    proxy = new Proxy(target, {
        getOwnPropertyDescriptor: function (targ, key) {
            print('getOwnPropertyDescriptor for ' + key);
            return {
                value: 'key-' + key,
                writable: false,
                enumerable: (key === 'nonEnumerable' ? false : true),
                configurable: true
            };
        },
        get: function (targ, key, recv) {
            return 'key-' + key;
        },
        enumerate: function () {
            print('enumerate should not be called');
            return [ 'quux' ];
        },
        ownKeys: function () {
            // ownKeys is called to decide what properties to serialize.
            // Non-enumerable properties are filtered out from JSON result.
            print('ownKeys should be called');
            return [ 'foo', 'quux', 'baz', 'nonEnumerable' ];
        }
    });
    print(JSON.stringify(proxy));
}

try {
    jsonStringifyOwnKeysProxyTest();
} catch (e) {
    print(e.stack || e);
}
