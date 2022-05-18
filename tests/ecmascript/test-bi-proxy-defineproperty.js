/*===
{"1001":345,"name":"target","foo":234} "foo" {"value":123,"writable":true,"enumerable":true,"configurable":true}
{"1001":345,"name":"target","foo":234} "1001" {"value":123,"writable":true,"enumerable":true,"configurable":true}
{"name":"target","foo":234} "foo" {"value":123,"writable":true,"enumerable":true,"configurable":false}
===*/

function test() {
    var P;

    P = new Proxy({ name: 'target', foo: 234, 1001: 345 }, {
        defineProperty: function (targ, key, desc) {
            print(JSON.stringify(targ), JSON.stringify(key), JSON.stringify(desc));
            return true;
        }
    });

    // The 'dummy' key gets filtered out because the argument descriptor is
    // first converted to a specification Property Descriptor and then back.
    // Similarly ToBoolean() conversions have been applied.
    Reflect.defineProperty(P, 'foo', { value: 123, dummy: true, writable: 1, enumerable: true, configurable: 'foo' });
    Reflect.defineProperty(P, 1001, { value: 123, dummy: true, writable: 1, enumerable: true, configurable: 'foo' });

    P = new Proxy({ name: 'target', foo: 234 }, {
        defineProperty: function (targ, key, desc) {
            print(JSON.stringify(targ), JSON.stringify(key), JSON.stringify(desc));
            return;  // falsy
        }
    });

    Reflect.defineProperty(P, 'foo', { value: 123, dummy: true, writable: 1, enumerable: true, configurable: false });
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
