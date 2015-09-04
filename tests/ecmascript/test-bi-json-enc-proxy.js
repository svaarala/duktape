/*
 *  Stringifying a Proxy should consult the underlying object.
 */

/*===
{"foo":234}
{"foo":345}
===*/

function jsonStringifyProxyTest() {
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

    // 'keys' trap is invoked to figure out keys to serialize
    // - JSON.stringify() calls internal function: http://www.ecma-international.org/ecma-262/6.0/#sec-serializejsonobject
    // - SerializeJSONObject() calls internal function: http://www.ecma-international.org/ecma-262/6.0/#sec-enumerableownnames
    // - EnumerableOwnNames() calls O.[[OwnPropertyKeys]]
    // - Proxy [[OwnPropertyKeys]] calls 'ownKeys' trap: http://www.ecma-international.org/ecma-262/6.0/#sec-proxy-object-internal-methods-and-internal-slots-ownpropertykeys
    // - XXX: must go through the post-processing to ensure the testcase is correct

    /* XXX: this test is disabled until the proper expect string is in place

    target = { foo: 123, bar: 234, quux: 345 };
    proxy = new Proxy(target, {
        get: function (targ, key, recv) {
            return 'key-' + key;
        },
        enumerate: function () {
            print('enumerate should not be called');
            return [];
        },
        ownKeys: function () {
            // ownKeys is called to decide what properties to serialize
            print('ownKeys should be called');
            return [ 'foo', 'quux', 'baz' ];
        }
    });
    print(JSON.stringify(proxy));
    */
}

try {
    jsonStringifyProxyTest();
} catch (e) {
    print(e.stack || e);
}
