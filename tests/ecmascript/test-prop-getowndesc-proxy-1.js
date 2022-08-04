/*===
getOwnPropertyDescriptor trap: string foo
{"value":123,"writable":true,"enumerable":false,"configurable":true}
getOwnPropertyDescriptor trap: string 2001
{"value":123,"writable":true,"enumerable":false,"configurable":true}
getOwnPropertyDescriptor trap: string foo
{"value":123,"writable":false,"enumerable":false,"configurable":true}
getOwnPropertyDescriptor trap: string foo
{"value":123,"writable":false,"enumerable":false,"configurable":false}
{"value":123,"writable":true,"enumerable":false,"configurable":false}
===*/

var target, P, pd;

// Full data descriptor, strkey.
target = {};
P = new Proxy(target, {
    getOwnPropertyDescriptor: function (target, key) {
        print('getOwnPropertyDescriptor trap:', typeof key, key);
        return { value: 123, writable: true, enumerable: false, configurable: true, foobar: 123 };
    }
});
pd = Object.getOwnPropertyDescriptor(P, 'foo');
print(JSON.stringify(pd));

// Full data descriptor, idxkey.
target = {};
P = new Proxy(target, {
    getOwnPropertyDescriptor: function (target, key) {
        print('getOwnPropertyDescriptor trap:', typeof key, key);
        return { value: 123, writable: true, enumerable: false, configurable: true, foobar: 123 };
    }
});
pd = Object.getOwnPropertyDescriptor(P, 2001);
print(JSON.stringify(pd));

// Partial data descriptor.
target = {};
P = new Proxy(target, {
    getOwnPropertyDescriptor: function (target, key) {
        print('getOwnPropertyDescriptor trap:', typeof key, key);
        return { value: 123, configurable: true };
    }
});
pd = Object.getOwnPropertyDescriptor(P, 'foo');
print(JSON.stringify(pd));

// Partial data descriptor.
target = {};
Object.defineProperty(target, 'foo', { value: 123, writable: false, enumerable: false, configurable: false });
P = new Proxy(target, {
    getOwnPropertyDescriptor: function (target, key) {
        print('getOwnPropertyDescriptor trap:', typeof key, key);
        return { value: 123 };
    }
});
pd = Object.getOwnPropertyDescriptor(P, 'foo');
print(JSON.stringify(pd));

// Passthru, no trap.
target = {};
Object.defineProperty(target, 'foo', { value: 123, writable: true, enumerable: false, configurable: false });
P = new Proxy(target, {});
pd = Object.getOwnPropertyDescriptor(P, 'foo');
print(JSON.stringify(pd));
