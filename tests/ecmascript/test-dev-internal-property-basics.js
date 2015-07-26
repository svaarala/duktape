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
for-in:  <255>quux
JSON.stringify: {"foo":1," <195><191>quux":4}
JX encoded: {foo:1," \xffquux":4}
Object.keys: foo, <255>quux
Object.getOwnPropertyNames: foo,bar, <255>quux
direct access: 3
===*/

function sanitizedPrint(x) {
    var buf, i, n;
    var tmp = '';

    buf = Duktape.Buffer(x);
    for (i = 0, n = buf.length; i < n; i++) {
        if (buf[i] < 0x20 || buf[i] > 0x7e || buf[i] == '<' || buf[i] == '>') {
            tmp += '<' + String(buf[i]) + '>';
        } else {
            tmp += String.fromCharCode(buf[i]);
        }
    }

    print(tmp);
}

function test() {
    var internalKey;
    var invalidUtf8Key;
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

    // The key \x20\xFFquux is invalid UTF-8 but not an internal string,
    // so the property will be enumerable.  Behavior for invalid UTF-8
    // varies.  For example, JSON.stringify() will encounter the invalid
    // UTF-8 initial byte \xFF and serialize it like it had encountered
    // the codepoint U+00FF (writing out the bytes c3 bf).
    invalidUtf8Key = Duktape.dec('hex', '20ff71757578');  // \x20\xFFquux
    obj[invalidUtf8Key] = 4;

    // For-in only lists enumerable keys
    for (k in obj) {
        sanitizedPrint('for-in: ' + k);
    }

    // JSON.stringify() also only lists enumerable keys
    sanitizedPrint('JSON.stringify: ' + JSON.stringify(obj));

    // Even the Duktape-specific JX format doesn't list internal keys
    sanitizedPrint('JX encoded: ' + Duktape.enc('jx', obj));

    // Object.keys() only returns enumerable keys (but not internal keys,
    // even if they're technically enumerable)
    sanitizedPrint('Object.keys: ' + Object.keys(obj));

    // Object.getOwnPropertyNames() returns also non-enumerable keys,
    // but not internal keys
    sanitizedPrint('Object.getOwnPropertyNames: ' + Object.getOwnPropertyNames(obj));

    // Direct property access to the internal property still works.
    sanitizedPrint('direct access: ' + obj[internalKey]);
}

try {
    test();
} catch (e) {
    print(e);
}
