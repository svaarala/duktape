// Object.defineProperties() normalizes the argument descriptors, but
// this must not affect the argument.

/*===
{"foo":{"value":123,"writable":true,"dummy":true},"bar":{"value":234,"writable":true,"foo":"bar"}}
{"foo":{"value":123,"writable":true,"dummy":true},"bar":{"value":234,"writable":true,"foo":"bar"}}
===*/

try {
    var descs = {
        foo: { value: 123, writable: true, dummy: true },
        bar: { value: 234, writable: true, foo: 'bar' }
    };

    var O = {};
    print(JSON.stringify(descs));
    Object.defineProperties(O, descs);
    print(JSON.stringify(descs));
} catch (e) {
    print(e.stack || e);
}
