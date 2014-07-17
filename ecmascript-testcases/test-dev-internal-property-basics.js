/*
 *  Since Duktape 0.12.0, user code can also employ internal properties.
 *  This testcase exercises a few basics of that.
 */

/*---
{
    "custom": true
}
---*/

/*===
for-in: foo
JSON.stringify: {"foo":1}
JX encoded: {foo:1}
Object.keys: foo
Object.getOwnPropertyNames: foo,bar
direct access: 3
===*/

function test() {
    var internalKey;
    var obj;

    // 'foo' is enumerable, writable, configurable
    var obj = { foo: 1 };

    // 'bar' is not enumerable
    Object.defineProperty(obj, 'bar', {
        value: 2, writable: true, enumerable: false, configurable: true
    });

    // Internal key \xFF\xFFabc is in principle enumerable, but because
    // internal keys have special behavior, it is never enumerated.
    internalKey = Duktape.dec('hex', 'ffff616263');  // \xFF\xFFabc
    obj[internalKey] = 3;

    // For-in only lists enumerable keys
    for (k in obj) {
        print('for-in:', k);
    }

    // JSON.stringify() also only lists enumerable keys
    print('JSON.stringify:', JSON.stringify(obj));

    // Even the Duktape-specific JX format doesn't list internal keys
    print('JX encoded:', Duktape.enc('jx', obj));

    // Object.keys() only returns enumerable keys (but not internal keys,
    // even if they're technically enumerable)
    print('Object.keys:', Object.keys(obj));

    // Object.getOwnPropertyNames() returns also non-enumerable keys,
    // but not internal keys
    print('Object.getOwnPropertyNames:', Object.getOwnPropertyNames(obj));

    // Direct property access to the internal property still works.
    print('direct access:', obj[internalKey]);
}

try {
    test();
} catch (e) {
    print(e);
}
