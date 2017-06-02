/*
 *  Error augmentation for constructor calls.
 */

/*---
{
    "custom": true
}
---*/

/*===
string
true
string
true
===*/

function test() {
    var t;

    t = new Error('aiee');
    print(typeof t.stack);
    print(t.lineNumber > 0);

    t = Reflect.construct(Error, [ 'aiee' ]);
    print(typeof t.stack);
    print(t.lineNumber > 0);
}

try {
    test();
} catch (e) {
    print(e.stack || e);
}
