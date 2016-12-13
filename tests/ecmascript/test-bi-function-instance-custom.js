/*
 *  Function instances, custom features
 */

/*---
{
    "custom": true
}
---*/

/*===
instance
true string
false false true
===*/

function functionInstanceTest() {
    var f = function test(a, b, c) {};
    var pd;

    print('fileName' in f, typeof f.fileName);
    pd = Object.getOwnPropertyDescriptor(f, 'fileName');
    print(pd.writable, pd.enumerable, pd.configurable);
}

try {
    print('instance');
    functionInstanceTest();
} catch (e) {
    print(e.stack || e);
}
