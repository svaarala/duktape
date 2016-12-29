/*
 *  ES2015 Section 7.3.17, step 8.b requires a TypeError if the return value for
 *  'ownKeys' trap contains non-string/non-symbol values.
 */

/*---
{
    "custom": true
}
---*/

/*===
["foo","bar"]
TypeError: invalid trap result
===*/

function test() {
    var P;
    var target = { foo: 123, bar: 123 };

    P = new Proxy(target, { ownKeys: function () { return ['foo', 'bar' ]; } });
    print(JSON.stringify(Object.keys(P)));

    P = new Proxy(target, { ownKeys: function () { return ['foo', 'bar', 123 ]; } });
    try {
        print(JSON.stringify(Object.keys(P)));
    } catch (e) {
        print(e);
    }
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
